#pragma once

#include <expected>
#include <string>
#include <vector>
#include <system_error>
#include "search_error.hpp"

// 搜索结果结构体
struct SearchResult {
    std::string title;
    std::string url;
    float relevance;
};

// 搜索服务类
class SearchService {
public:
    // 使用std::expected返回搜索结果或搜索错误
    std::expected<std::vector<SearchResult>, std::error_code> search(const std::string& query);
    
    // 示例：使用文件名搜索文件内容（演示系统标准错误）
    std::expected<std::string, std::error_code> search_file(const std::string& filename);
    
    // 模拟索引管理，返回标准系统错误或自定义错误
    std::expected<bool, std::error_code> rebuild_index();
    
    // 设置搜索配置，演示多种错误处理方式
    std::expected<void, std::error_code> configure(const std::string& config_json);
}; 