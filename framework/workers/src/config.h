#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// 简化的配置类
class Config {
public:
    Config();
    ~Config() = default;

    bool load(const std::string& configFile);
    
    // 配置项
    int getPort() const { return port_; }
    int getWorkerCount() const { return workerCount_; }
    int getMaxConnections() const { return maxConnections_; }
    
private:
    int port_ = 8080;           // 监听端口
    int workerCount_ = 4;       // worker进程数量
    int maxConnections_ = 1024; // 最大连接数
};

#endif // CONFIG_H 