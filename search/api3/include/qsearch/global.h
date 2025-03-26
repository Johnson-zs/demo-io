#pragma once

#include <QtCore/qglobal.h>

#if defined(QSEARCH_LIBRARY)
#  define QSEARCH_EXPORT Q_DECL_EXPORT
#else
#  define QSEARCH_EXPORT Q_DECL_IMPORT
#endif

namespace QSearch {
    // 全局常量定义
    constexpr int API_VERSION = 1;
    
    // 定义搜索库的全局功能标志
    namespace Features {
        constexpr const char* PINYIN_SEARCH = "pinyin_search";
        constexpr const char* FUZZY_SEARCH = "fuzzy_search";
        constexpr const char* REGEX_SEARCH = "regex_search";
        constexpr const char* CONTENT_PREVIEW = "content_preview";
        constexpr const char* AI_ASSISTED = "ai_assisted";
    }
} 