#include "master.h"
#include "worker.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/mman.h>

// 静态成员变量初始化
Master* Master::instance_ = nullptr;

Master::Master(const Config& config)
    : config_(config), listenFd_(-1), acceptMutex_(nullptr), running_(false) {
    // 设置全局实例指针，用于信号处理
    setInstance(this);
}

Master::~Master() {
    stop();
    
    // 关闭监听套接字
    if (listenFd_ >= 0) {
        close(listenFd_);
        listenFd_ = -1;
    }
    
    // 释放互斥锁内存
    if (acceptMutex_) {
        munmap(acceptMutex_, sizeof(std::atomic<int>));
        acceptMutex_ = nullptr;
    }
    
    // 清除全局实例指针
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

bool Master::init() {
    // 创建监听套接字
    if (!createListenSocket()) {
        return false;
    }
    
    // 创建共享内存中的互斥锁
    void* ptr = mmap(nullptr, sizeof(std::atomic<int>), PROT_READ | PROT_WRITE, 
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "创建共享内存失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    acceptMutex_ = new(ptr) std::atomic<int>(0);
    return true;
}

bool Master::createListenSocket() {
    // 创建套接字
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        std::cerr << "创建套接字失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 设置地址重用
    int reuse = 1;
    if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "设置SO_REUSEADDR失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 设置非阻塞
    if (!setNonBlocking(listenFd_)) {
        return false;
    }
    
    // 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(config_.getPort());
    
    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "绑定地址失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 开始监听
    if (listen(listenFd_, SOMAXCONN) < 0) {
        std::cerr << "监听失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    std::cout << "监听套接字创建成功，端口: " << config_.getPort() << std::endl;
    return true;
}

bool Master::setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "获取套接字标志失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "设置套接字为非阻塞失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool Master::start() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGCHLD, signalHandler);
    
    // 创建Worker进程
    if (!createWorkers()) {
        return false;
    }
    
    running_ = true;
    std::cout << "Master进程启动成功 (PID: " << getpid() << ")" << std::endl;
    return true;
}

bool Master::createWorkers() {
    int workerCount = config_.getWorkerCount();
    std::cout << "创建 " << workerCount << " 个Worker进程..." << std::endl;
    
    for (int i = 0; i < workerCount; ++i) {
        pid_t pid = createWorker(i);
        if (pid < 0) {
            std::cerr << "创建Worker[" << i << "]失败" << std::endl;
            return false;
        }
        
        workerPids_.push_back(pid);
        std::cout << "Worker[" << i << "] 创建成功 (PID: " << pid << ")" << std::endl;
    }
    
    return true;
}

pid_t Master::createWorker(int workerId) {
    pid_t pid = fork();
    
    if (pid < 0) {
        // 创建失败
        std::cerr << "fork失败: " << strerror(errno) << std::endl;
        return -1;
    } else if (pid == 0) {
        // 子进程 (Worker)
        
        // 创建Worker并运行
        Worker worker(workerId, listenFd_, config_.getMaxConnections());
        worker.setAcceptMutex(acceptMutex_);
        
        if (worker.init()) {
            worker.run();
        }
        
        // Worker结束
        exit(0);
    }
    
    // 父进程 (Master)
    return pid;
}

void Master::stop() {
    running_ = false;
    
    // 向所有Worker发送终止信号
    for (pid_t pid : workerPids_) {
        if (pid > 0) {
            kill(pid, SIGTERM);
            std::cout << "向Worker进程(PID: " << pid << ")发送SIGTERM信号" << std::endl;
        }
    }
    
    // 等待Worker进程
    waitForWorkers();
    
    // 清空Worker进程列表
    workerPids_.clear();
}

void Master::waitForWorkers() {
    for (size_t i = 0; i < workerPids_.size(); ++i) {
        if (workerPids_[i] > 0) {
            int status;
            pid_t pid = waitpid(workerPids_[i], &status, 0);
            
            if (pid > 0) {
                std::cout << "Worker进程(PID: " << pid << ")已退出，状态: " 
                          << (WIFEXITED(status) ? WEXITSTATUS(status) : -1) << std::endl;
                workerPids_[i] = -1;
            }
        }
    }
}

bool Master::reload() {
    std::cout << "重新加载配置..." << std::endl;
    // 在这里可以重新加载配置，重启Worker进程等
    
    // 先停止旧的Worker进程
    stop();
    
    // 创建新的Worker进程
    if (!createWorkers()) {
        return false;
    }
    
    std::cout << "配置已重新加载" << std::endl;
    return true;
}

void Master::signalHandler(int sig) {
    if (!instance_) {
        return;
    }
    
    switch (sig) {
        case SIGINT:
        case SIGTERM: {
            std::cout << "接收到终止信号(" << sig << ")，开始关闭..." << std::endl;
            instance_->stop();
            exit(0);
            break;
        }
        case SIGHUP: {
            std::cout << "接收到HUP信号，重新加载配置..." << std::endl;
            instance_->reload();
            break;
        }
        case SIGCHLD: {
            // 处理子进程退出
            int status;
            pid_t pid;
            while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                std::cout << "子进程(PID: " << pid << ")退出，状态: " 
                          << (WIFEXITED(status) ? WEXITSTATUS(status) : -1) << std::endl;
                
                // 从进程列表中移除
                auto& pids = instance_->workerPids_;
                for (size_t i = 0; i < pids.size(); ++i) {
                    if (pids[i] == pid) {
                        pids[i] = -1;
                        break;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
} 