#include "config.h"
#include <iostream>
#include <fstream>
#include <string>

Config::Config() {
    // 默认构造，使用头文件中的默认值
}

bool Config::load(const std::string& configFile) {
    // 简化的配置加载
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << configFile << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // 简单解析，实际项目中应该更加健壮
        if (line.find("port=") == 0) {
            port_ = std::stoi(line.substr(5));
        } else if (line.find("worker_processes=") == 0) {
            workerCount_ = std::stoi(line.substr(17));
        } else if (line.find("max_connections=") == 0) {
            maxConnections_ = std::stoi(line.substr(16));
        }
    }
    
    std::cout << "配置加载完成:" << std::endl;
    std::cout << "- 端口: " << port_ << std::endl;
    std::cout << "- Worker进程数: " << workerCount_ << std::endl;
    std::cout << "- 最大连接数: " << maxConnections_ << std::endl;
    
    return true;
} 