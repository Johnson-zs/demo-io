#ifndef DFM6_SEARCH_GLOBAL_H
#define DFM6_SEARCH_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DFM6_SEARCH_LIBRARY)
#    define DFM6_SEARCH_EXPORT Q_DECL_EXPORT
#else
#    define DFM6_SEARCH_EXPORT Q_DECL_IMPORT
#endif

namespace DFM6 {
namespace Search {

// 搜索类型枚举
enum class SearchType {
    FileName,   // 文件名搜索
    Content,   // 内容搜索
    Custom = 50   // 用户自定义搜索类型
};

// 搜索状态枚举
enum class SearchStatus {
    Ready,   // 准备就绪
    Searching,   // 正在搜索
    Paused,   // 已暂停
    Finished,   // 已完成
    Cancelled,   // 已取消
    Error   // 错误
};

// 搜索方式枚举
enum class SearchMethod {
    Indexed,   // 索引搜索
    Realtime   // 实时搜索
};

}   // namespace Search
}   // namespace DFM6

#endif   // DFM6_SEARCH_GLOBAL_H
