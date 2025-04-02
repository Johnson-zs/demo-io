#include "eventlistener.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>   // close()

#include <QDebug>
#include <QString>
#include <QDateTime>

#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>

#include "utils/logger.h"

/* protocol family */
#define VFSMONITOR_FAMILY_NAME "vfsmonitor"

/* attributes */
enum {
    VFSMONITOR_A_UNSPEC,
    VFSMONITOR_A_ACT,
    VFSMONITOR_A_COOKIE,
    VFSMONITOR_A_MAJOR,
    VFSMONITOR_A_MINOR,
    VFSMONITOR_A_PATH,
    __VFSMONITOR_A_MAX,
};
#define VFSMONITOR_A_MAX (__VFSMONITOR_A_MAX - 1)

/* commands */
enum {
    VFSMONITOR_C_UNSPEC,
    VFSMONITOR_C_NOTIFY,
    __VFSMONITOR_C_MAX,
};
#define VFSMONITOR_C_MAX (__VFSMONITOR_C_MAX - 1)

/* multicast group */
#define VFSMONITOR_MCG_DENTRY_NAME VFSMONITOR_FAMILY_NAME "_de"

constexpr int epoll_size = 10;
static nla_policy vfs_policy[VFSMONITOR_A_MAX + 1];

// Parser class to extract values from netlink attributes
class NlaParser
{
public:
    explicit NlaParser(nlattr **attrs)
        : attrs_(attrs) { }

    template<typename T>
    std::optional<T> getValue(int attrType) const
    {
        return std::nullopt;
    }

private:
    nlattr **attrs_;
};

// Template specializations for different types
template<>
std::optional<uint8_t> NlaParser::getValue<uint8_t>(int attrType) const
{
    if (attrs_[attrType]) {
        return nla_get_u8(attrs_[attrType]);
    }
    return std::nullopt;
}

template<>
std::optional<uint16_t> NlaParser::getValue<uint16_t>(int attrType) const
{
    if (attrs_[attrType]) {
        return nla_get_u16(attrs_[attrType]);
    }
    return std::nullopt;
}

template<>
std::optional<uint32_t> NlaParser::getValue<uint32_t>(int attrType) const
{
    if (attrs_[attrType]) {
        return nla_get_u32(attrs_[attrType]);
    }
    return std::nullopt;
}

template<>
std::optional<char *> NlaParser::getValue<char *>(int attrType) const
{
    if (attrs_[attrType]) {
        return nla_get_string(attrs_[attrType]);
    }
    return std::nullopt;
}

EventListener::EventListener(QObject *parent)
    : QObject(parent), connected_(false), stopFd_(-1), timeout_(-1)
{

    auto cleanAndAbort = [this] {
        disconnect(mcsk_);
        std::abort();
    };

    // Connect to netlink
    connected_ = connect(mcsk_);
    if (!connected_) {
        Logger::logError("Failed to connect to generic netlink");
        cleanAndAbort();
    }

    // Disable sequence checks for asynchronous multicast messages
    nl_socket_disable_seq_check(mcsk_);
    nl_socket_disable_auto_ack(mcsk_);

    // Resolve the multicast group
    int mcgrp = genl_ctrl_resolve_grp(mcsk_, VFSMONITOR_FAMILY_NAME, VFSMONITOR_MCG_DENTRY_NAME);
    if (mcgrp < 0) {
        Logger::logError("Failed to resolve generic netlink multicast group");
        cleanAndAbort();
    }

    // Join the multicast group
    int ret = nl_socket_add_membership(mcsk_, mcgrp);
    if (ret < 0) {
        Logger::logError("Failed to join multicast group");
        cleanAndAbort();
    }

    if (!setCallback(mcsk_, EventListener::eventHandler)) {
        Logger::logError("Failed to set callback");
        cleanAndAbort();
    }

    stopFd_ = eventfd(0, EFD_NONBLOCK);
    if (stopFd_ == -1) {
        Logger::logError("Failed to create eventfd");
        cleanAndAbort();
    }

    // Initialize policy
    vfs_policy[VFSMONITOR_A_ACT].type = NLA_U8;
    vfs_policy[VFSMONITOR_A_COOKIE].type = NLA_U32;
    vfs_policy[VFSMONITOR_A_MAJOR].type = NLA_U16;
    vfs_policy[VFSMONITOR_A_MINOR].type = NLA_U8;
    vfs_policy[VFSMONITOR_A_PATH].type = NLA_NUL_STRING;
    vfs_policy[VFSMONITOR_A_PATH].maxlen = 4096;
}

EventListener::~EventListener()
{
    stopListening();
    disconnect(mcsk_);

    if (stopFd_ != -1) {
        ::close(stopFd_);
    }
}

bool EventListener::startListening()
{
    Logger::logInfo("Starting to listen for file system events");
    int epFd = epoll_create1(0);
    if (epFd < 0) {
        Logger::logError("Epoll creation failed");
        return false;
    }

    int mcskFd = getFd(mcsk_);
    auto epEvents = std::make_unique<epoll_event[]>(epoll_size);
    epoll_event event[2];
    event[0].events = EPOLLIN;
    event[0].data.fd = mcskFd;
    event[1].events = EPOLLIN;
    event[1].data.fd = stopFd_;
    epoll_ctl(epFd, EPOLL_CTL_ADD, mcskFd, &event[0]);
    epoll_ctl(epFd, EPOLL_CTL_ADD, stopFd_, &event[1]);

    bool running = true;
    while (running) {
        int eventCnt = epoll_wait(epFd, epEvents.get(), epoll_size, timeout_);
        if (eventCnt == -1) {
            if (errno == EINTR) {
                continue;
            }
            Logger::logError(QString("epoll_wait() error: %1 (errno: %2)")
                                     .arg(strerror(errno))
                                     .arg(errno));
            break;
        }

        for (int i = 0; i < eventCnt; ++i) {
            if (epEvents[i].data.fd == mcskFd) {
                nl_recvmsgs_default(mcsk_);
            } else if (epEvents[i].data.fd == stopFd_) {
                uint64_t u;
                [[maybe_unused]] auto _ = read(stopFd_, &u, sizeof(u));
                running = false;
                break;
            }
        }
    }

    ::close(epFd);
    return true;
}

void EventListener::asyncListen()
{
    if (listeningThread_.joinable()) {
        Logger::logInfo("Listening thread is already running");
        return;
    }

    Logger::logInfo("Starting async listening for file system events");
    listeningThread_ = std::thread([this]() {
        startListening();
    });
}

void EventListener::stopListening()
{
    if (stopFd_ != -1) {
        uint64_t u = 1;
        [[maybe_unused]] auto _ = write(stopFd_, &u, sizeof(u));
    }

    if (listeningThread_.joinable()) {
        listeningThread_.join();
        Logger::logInfo("Listening thread has stopped");
    }
}

bool EventListener::connect(nl_sock_ptr &sk) const
{
    sk = nl_socket_alloc();
    return sk ? genl_connect(sk) == 0 : false;
}

void EventListener::disconnect(nl_sock_ptr &sk) const
{
    nl_socket_free(sk);
}

bool EventListener::setCallback(nl_sock_ptr &sk, nl_recvmsg_msg_cb_t func)
{
    return nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, func, this) == 0;
}

int EventListener::getFd(nl_sock_ptr &sk) const
{
    return nl_socket_get_fd(sk);
}

void EventListener::forwardEventToHandler(FSEvent event)
{
    emit eventReceived(event);
}

int EventListener::eventHandler(nl_msg_ptr msg, void *arg)
{
    nlattr *tb[VFSMONITOR_A_MAX + 1];
    int err = genlmsg_parse(nlmsg_hdr(msg), 0, tb, VFSMONITOR_A_MAX, vfs_policy);
    if (err < 0) {
        Logger::logError(QString("Unable to parse the message: %1").arg(strerror(-err)));
        return NL_SKIP;
    }

    if (!tb[VFSMONITOR_A_PATH]) {
        Logger::logError("Attributes missing from the message");
        return NL_SKIP;
    }

    NlaParser parser(tb);
    auto act = parser.getValue<uint8_t>(VFSMONITOR_A_ACT);
    auto cookie = parser.getValue<uint32_t>(VFSMONITOR_A_COOKIE);
    auto major = parser.getValue<uint16_t>(VFSMONITOR_A_MAJOR);
    auto minor = parser.getValue<uint8_t>(VFSMONITOR_A_MINOR);
    auto src = parser.getValue<char *>(VFSMONITOR_A_PATH);

    if (!act || !cookie || !major || !minor || !src) {
        Logger::logError("Attributes missing from the message");
        return NL_SKIP;
    }

    FSEvent event;
    event.type = static_cast<FSEvent::EventType>(*act);
    event.cookie = *cookie;
    event.major = *major;
    event.minor = *minor;
    event.path = QString::fromUtf8(*src);
    event.timestamp = QDateTime::currentDateTime();

    auto listener = static_cast<EventListener *>(arg);
    listener->forwardEventToHandler(std::move(event));
    return NL_OK;
}
