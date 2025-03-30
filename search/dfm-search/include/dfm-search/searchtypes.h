#ifndef DFM_SEARCH_TYPES_H
#define DFM_SEARCH_TYPES_H

#include <QFlags>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariant>
#include <QMap>
#include <functional>
#include <optional>
#include <variant>

namespace DFM {
namespace Search {

/**
 * @brief 搜索类型枚举
 */
enum class SearchType {
    FileName,       // 文件名搜索
    Fulltext,       // 全文搜索  
    OcrImage,       // OCR 图像搜索
    Application,    // 应用搜索
    Custom          // 自定义搜索类型
};

/**
 * @brief 搜索模式枚举
 */
enum class SearchMode {
    Indexed,        // 使用预建索引搜索
    Realtime        // 实时搜索
};

/**
 * @brief 搜索选项标志
 */
enum SearchOptionFlag {
    None              = 0,
    CaseSensitive     = 1 << 0,   // 大小写敏感
    PinyinMatch       = 1 << 1,   // 拼音匹配
    RegexPattern      = 1 << 2,   // 正则表达式支持
    FuzzyMatch        = 1 << 3,   // 模糊匹配
    ExactMatch        = 1 << 4,   // 精确匹配
    RecursiveSearch   = 1 << 5,   // 递归搜索
    FollowSymlinks    = 1 << 6,   // 跟随符号链接
    HiddenFiles       = 1 << 7    // 包含隐藏文件
};
Q_DECLARE_FLAGS(SearchOptions, SearchOptionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchOptions)

/**
 * @brief 搜索范围限制
 */
struct SearchScope {
    QStringList includePaths;     // 包含路径
    QStringList excludePaths;     // 排除路径
    QStringList includePatterns;  // 包含模式 (如 *.txt)
    QStringList excludePatterns;  // 排除模式 (如 *.tmp)

    // 可选的时间范围限制
    std::optional<QDateTime> modifiedAfter;
    std::optional<QDateTime> modifiedBefore;
    std::optional<QDateTime> createdAfter;
    std::optional<QDateTime> createdBefore;
    
    // 可选的大小限制 (字节)
    std::optional<qint64> minSize;
    std::optional<qint64> maxSize;
};

/**
 * @brief 布尔逻辑操作
 */
enum class LogicalOperator {
    And,    // 与操作
    Or,     // 或操作
    Not     // 非操作
};

/**
 * @brief 搜索条件组合
 */
struct SearchCondition {
    QString term;                       // 搜索词
    LogicalOperator logicOp = LogicalOperator::And; // 与其他条件的组合方式
};

/**
 * @brief 搜索排序方式
 */
enum class SortBy {
    Relevance,     // 相关度排序
    Name,          // 名称排序
    Path,          // 路径排序
    Size,          // 大小排序
    ModifiedTime,  // 修改时间排序
    CreatedTime    // 创建时间排序  
};

/**
 * @brief 排序方向
 */
enum class SortOrder {
    Ascending,    // 升序
    Descending    // 降序
};

/**
 * @brief 结果分页信息
 */
struct Pagination {
    int offset = 0;         // 起始偏移
    int limit = 100;        // 限制数量
};

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_TYPES_H 