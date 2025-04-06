#include "search_error.hpp"

// 实现单例模式获取类别实例
const search_error_category& search_error_category::instance() {
    static search_error_category instance;
    return instance;
}

// 实现返回错误类别名称
const char* search_error_category::name() const noexcept {
    return "search";
}

// 实现返回错误对应的消息
std::string search_error_category::message(int ev) const {
    switch (static_cast<search_errc>(ev)) {
        case search_errc::success:
            return "搜索成功";
        case search_errc::empty_query:
            return "搜索查询为空";
        case search_errc::query_too_long:
            return "搜索查询过长";
        case search_errc::index_unavailable:
            return "搜索索引不可用";
        case search_errc::too_many_results:
            return "搜索结果过多";
        case search_errc::internal_error:
            return "搜索内部错误";
        default:
            return "未知搜索错误";
    }
} 