#include <iostream>
#include <string>
#include <iomanip>
#include "search_service.hpp"

// 显示错误信息辅助函数
void print_error(const std::error_code& ec) {
    std::cout << "错误代码: " << ec.value() << std::endl;
    std::cout << "错误类别: " << ec.category().name() << std::endl;
    std::cout << "错误消息: " << ec.message() << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

int main() {
    std::cout << "C++23 错误处理示例 - 使用 std::expected 和 std::error_category" << std::endl;
    std::cout << "==========================================================" << std::endl;
    
    SearchService service;
    
    // 演示1: 搜索功能（自定义错误）
    std::cout << "\n演示1: 基本搜索功能" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // 正常搜索
    {
        auto result = service.search("C++ programming");
        if (result) {
            std::cout << "搜索成功！找到 " << result->size() << " 个结果:" << std::endl;
            for (const auto& item : *result) {
                std::cout << " - " << std::left << std::setw(20) << item.title 
                          << " | 相关度: " << item.relevance << std::endl;
            }
        } else {
            print_error(result.error());
        }
    }
    
    // 空查询错误
    {
        std::cout << "\n空查询测试:" << std::endl;
        auto result = service.search("");
        if (!result) {
            print_error(result.error());
        }
    }
    
    // 查询过长错误
    {
        std::cout << "\n查询过长测试:" << std::endl;
        std::string long_query(150, 'a');
        auto result = service.search(long_query);
        if (!result) {
            print_error(result.error());
        }
    }
    
    // 演示2: 文件搜索（标准系统错误）
    std::cout << "\n演示2: 文件搜索功能（系统标准错误）" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // 文件不存在错误
    {
        auto result = service.search_file("不存在的文件.txt");
        if (result) {
            std::cout << "文件内容: " << *result << std::endl;
        } else {
            print_error(result.error());
        }
    }
    
    // 参数无效错误
    {
        auto result = service.search_file("");
        if (!result) {
            print_error(result.error());
        }
    }
    
    // 演示3: 混合错误处理
    std::cout << "\n演示3: 索引重建（混合错误处理）" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // 尝试多次重建索引以演示各种错误
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n尝试 #" << (i + 1) << " 重建索引:" << std::endl;
        auto result = service.rebuild_index();
        if (result) {
            std::cout << "索引重建成功！" << std::endl;
        } else {
            print_error(result.error());
        }
    }
    
    // 演示4: void返回值错误处理
    std::cout << "\n演示4: 配置服务（void返回值错误处理）" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // 有效配置
    {
        auto result = service.configure("{\"max_results\": 10}");
        if (result) {
            std::cout << "配置成功！" << std::endl;
        } else {
            print_error(result.error());
        }
    }
    
    // 无效配置
    {
        auto result = service.configure("无效的配置");
        if (!result) {
            print_error(result.error());
        }
    }
    
    // 空配置
    {
        auto result = service.configure("");
        if (!result) {
            print_error(result.error());
        }
    }
    
    std::cout << "\n演示完成！" << std::endl;
    return 0;
} 