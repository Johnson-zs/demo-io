#include "config.h"
#include "master.h"
#include <iostream>
#include <string>

void printUsage(const char* programName) {
    std::cout << "使用方法: " << programName << " [配置文件路径]" << std::endl;
    std::cout << "如果未指定配置文件路径，将使用默认配置。" << std::endl;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    std::string configFile;
    if (argc > 1) {
        configFile = argv[1];
    }
    
    // 创建配置
    Config config;
    
    // 如果指定了配置文件，则加载配置
    if (!configFile.empty()) {
        if (!config.load(configFile)) {
            std::cerr << "加载配置文件失败: " << configFile << std::endl;
            return 1;
        }
    } else {
        std::cout << "使用默认配置:" << std::endl;
        std::cout << "- 端口: " << config.getPort() << std::endl;
        std::cout << "- Worker进程数: " << config.getWorkerCount() << std::endl;
        std::cout << "- 最大连接数: " << config.getMaxConnections() << std::endl;
    }
    
    // 创建Master
    Master master(config);
    
    // 初始化Master
    if (!master.init()) {
        std::cerr << "初始化Master失败" << std::endl;
        return 1;
    }
    
    // 启动Master
    if (!master.start()) {
        std::cerr << "启动Master失败" << std::endl;
        return 1;
    }
    
    // 等待Worker进程
    master.waitForWorkers();
    
    return 0;
} 