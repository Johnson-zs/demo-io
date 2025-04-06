#pragma once

#include <system_error>
#include <string>

// 搜索错误代码枚举
enum class search_errc {
    success = 0,
    empty_query = 1,
    query_too_long = 2,
    index_unavailable = 3,
    too_many_results = 4,
    internal_error = 5
};

// 声明搜索错误类别
class search_error_category : public std::error_category {
public:
    // 单例模式获取类别实例
    static const search_error_category& instance();
    
    // 返回错误类别名称
    const char* name() const noexcept override;
    
    // 返回错误对应的消息
    std::string message(int ev) const override;
    
private:
    search_error_category() = default;
};

// 将搜索错误代码转换为std::error_code
inline std::error_code make_error_code(search_errc e) {
    return {static_cast<int>(e), search_error_category::instance()};
}

// 使自定义错误码与std::error_code兼容
namespace std {
    template <>
    struct is_error_code_enum<search_errc> : true_type {};
} 