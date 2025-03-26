#pragma once

#include <QtCore/qglobal.h>

#if defined(QTSEARCHKIT_LIBRARY)
#  define QTSEARCHKIT_EXPORT Q_DECL_EXPORT
#else
#  define QTSEARCHKIT_EXPORT Q_DECL_IMPORT
#endif

namespace QtSearchKit {

// 统一的搜索类型定义
enum class SearchType {
    FileName,       // 文件名搜索
    FullText,       // 全文搜索
    Image,          // 图片搜索
    Application,    // 应用搜索
    Contact,        // 联系人搜索
    Help,           // 帮助文档搜索
    Vector,         // 向量搜索
    Custom          // 自定义搜索类型
};

// 索引状态
enum class IndexStatus {
    NotInitialized,
    Initializing,
    Ready,
    Indexing,
    Updating,
    Paused,
    Error
};

// 搜索结果相关性
enum class Relevance {
    Low,
    Medium,
    High,
    Exact
};

// 搜索模式
enum class SearchMode {
    Simple,         // 简单搜索
    Boolean,        // 布尔搜索 (AND, OR, NOT)
    Wildcard,       // 通配符搜索 (*, ?)
    Regex,          // 正则表达式搜索
    Fuzzy,          // 模糊搜索
    Phonetic,       // 语音搜索 (包括拼音)
    Semantic        // 语义搜索
};

// 标准搜索选项名称
namespace SearchOptions {
    // 通用选项
    constexpr const char* CaseSensitive = "case_sensitive";       // bool
    constexpr const char* WholeWord = "whole_word";               // bool
    constexpr const char* SearchMode = "search_mode";             // SearchMode
    constexpr const char* MaxResults = "max_results";             // int
    constexpr const char* SortBy = "sort_by";                     // QString
    constexpr const char* SortOrder = "sort_order";               // Qt::SortOrder
    
    // 文件搜索特定选项
    constexpr const char* IncludeHidden = "include_hidden";       // bool
    constexpr const char* FollowSymlinks = "follow_symlinks";     // bool
    constexpr const char* MaxDepth = "max_depth";                 // int
    constexpr const char* FileTypes = "file_types";               // QStringList
    constexpr const char* MinSize = "min_size";                   // qint64 (bytes)
    constexpr const char* MaxSize = "max_size";                   // qint64 (bytes)
    constexpr const char* ModifiedAfter = "modified_after";       // QDateTime
    constexpr const char* ModifiedBefore = "modified_before";     // QDateTime
    
    // 全文搜索特定选项
    constexpr const char* Encoding = "encoding";                  // QString
    constexpr const char* MaxLineLength = "max_line_length";      // int
    constexpr const char* MaxContextLines = "max_context_lines";  // int
    constexpr const char* MatchInContext = "match_in_context";    // bool
    
    // 模糊搜索选项
    constexpr const char* FuzzyThreshold = "fuzzy_threshold";     // float (0.0-1.0)
    
    // 拼音搜索选项
    constexpr const char* EnablePinyin = "enable_pinyin";         // bool
    constexpr const char* PinyinMode = "pinyin_mode";             // int (全拼、首字母等)
    
    // 向量搜索选项
    constexpr const char* VectorDimension = "vector_dimension";   // int
    constexpr const char* SimilarityMetric = "similarity_metric"; // QString
    constexpr const char* SimilarityThreshold = "similarity_threshold"; // float
}

} // namespace QtSearchKit 