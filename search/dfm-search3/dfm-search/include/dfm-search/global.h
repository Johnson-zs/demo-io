#pragma once

#include <QtCore/qglobal.h>

#if defined(DFMSEARCH_LIBRARY)
#  define DFMSEARCH_EXPORT Q_DECL_EXPORT
#else
#  define DFMSEARCH_EXPORT Q_DECL_IMPORT
#endif

namespace DFMSearch {

// 搜索类型枚举
enum class SearchType {
    FileName,       // 文件名搜索
    FileContent,    // 文件内容搜索
    Application,    // 应用程序搜索
    Custom          // 自定义搜索类型
};

// 搜索机制枚举
enum class SearchMechanism {
    Indexed,        // 基于索引的搜索
    RealTime        // 实时搜索
};

// 搜索状态枚举
enum class SearchStatus {
    Ready,          // 就绪
    Searching,      // 搜索中
    Paused,         // 暂停
    Completed,      // 完成
    Error           // 错误
};

// 搜索选项标志
enum class SearchFlag {
    None              = 0x00,
    CaseSensitive     = 0x01,
    PinyinSupport     = 0x02,
    RegexSupport      = 0x04,
    FuzzyMatch        = 0x08,
    ExactMatch        = 0x10
};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchFlags)

// 搜索结果类型
enum class ResultType {
    File,           // 文件结果
    Content,        // 内容结果
    Application,    // 应用程序结果
    Custom          // 自定义结果
};

} // namespace DFMSearch 