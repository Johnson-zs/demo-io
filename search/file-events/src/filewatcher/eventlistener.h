#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <atomic>
#include <functional>
#include <thread>

#include <netlink/attr.h>
#include <netlink/handlers.h>

#include <QObject>

#include "utils/fsevents.h"

using nl_sock_ptr = nl_sock*;
using nl_msg_ptr = nl_msg*;

class EventListener : public QObject {
    Q_OBJECT
public:
    explicit EventListener(QObject* parent = nullptr);
    ~EventListener();

    bool startListening();
    void asyncListen();
    void stopListening();

signals:
    void eventReceived(const FSEvent& event);

private:
    bool connect(nl_sock_ptr& sk) const;
    void disconnect(nl_sock_ptr& sk) const;
    bool setCallback(nl_sock_ptr& sk, nl_recvmsg_msg_cb_t func);
    int getFd(nl_sock_ptr& sk) const;

    void forwardEventToHandler(FSEvent event);
    static int eventHandler(nl_msg_ptr msg, void* arg);

private:
    nl_sock_ptr mcsk_;
    bool connected_;
    int stopFd_;
    int fam_;
    int timeout_;
    std::thread listeningThread_;
};

#endif // EVENTLISTENER_H 