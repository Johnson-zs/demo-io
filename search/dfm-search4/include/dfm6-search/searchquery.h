#ifndef DFM6_SEARCH_QUERY_H
#define DFM6_SEARCH_QUERY_H

#include <dfm6-search/global.h>
#include <QString>
#include <QStringList>
#include <memory>

namespace DFM6 {
namespace Search {

class SearchQueryData;

/**
 * @brief 搜索查询类
 * 
 * 封装搜索查询条件
 */
class DFM6_SEARCH_EXPORT SearchQuery
{
public:
    /**
     * @brief 查询类型枚举
     */
    enum class Type {
        Simple,     // 简单查询
        Boolean,    // 布尔查询（与、或、非）
        Wildcard,   // 通配符查询
        Fuzzy,      // 模糊查询
        Regex       // 正则表达式查询
    };
    
    /**
     * @brief 布尔操作符枚举
     */
    enum class BooleanOperator {
        AND,        // 逻辑与
        OR,         // 逻辑或
        NOT         // 逻辑非
    };
    
    /**
     * @brief 构造函数
     */
    SearchQuery();
    
    /**
     * @brief 使用关键词构造
     */
    explicit SearchQuery(const QString &keyword);
    
    /**
     * @brief 使用关键词和类型构造
     */
    SearchQuery(const QString &keyword, Type type);
    
    /**
     * @brief 复制构造函数
     */
    SearchQuery(const SearchQuery &other);
    
    /**
     * @brief 移动构造函数
     */
    SearchQuery(SearchQuery &&other) noexcept;
    
    /**
     * @brief 析构函数
     */
    ~SearchQuery();
    
    /**
     * @brief 赋值操作符
     */
    SearchQuery& operator=(const SearchQuery &other);
    
    /**
     * @brief 移动赋值操作符
     */
    SearchQuery& operator=(SearchQuery &&other) noexcept;
    
    /**
     * @brief 获取关键词
     */
    QString keyword() const;
    
    /**
     * @brief 设置关键词
     */
    void setKeyword(const QString &keyword);
    
    /**
     * @brief 获取查询类型
     */
    Type type() const;
    
    /**
     * @brief 设置查询类型
     */
    void setType(Type type);
    
    /**
     * @brief 获取布尔操作符
     */
    BooleanOperator booleanOperator() const;
    
    /**
     * @brief 设置布尔操作符
     */
    void setBooleanOperator(BooleanOperator op);
    
    /**
     * @brief 添加子查询（用于布尔查询）
     */
    void addSubQuery(const SearchQuery &query);
    
    /**
     * @brief 获取子查询列表
     */
    QList<SearchQuery> subQueries() const;
    
    /**
     * @brief 清空子查询
     */
    void clearSubQueries();
    
    /**
     * @brief 创建简单查询
     */
    static SearchQuery createSimpleQuery(const QString &keyword);
    
    /**
     * @brief 创建布尔查询
     */
    static SearchQuery createBooleanQuery(const QStringList &keywords, BooleanOperator op = BooleanOperator::AND);
    
    /**
     * @brief 创建通配符查询
     */
    static SearchQuery createWildcardQuery(const QString &pattern);
    
    /**
     * @brief 创建模糊查询
     */
    static SearchQuery createFuzzyQuery(const QString &keyword);
    
    /**
     * @brief 创建正则表达式查询
     */
    static SearchQuery createRegexQuery(const QString &pattern);

private:
    std::unique_ptr<SearchQueryData> d;  // PIMPL
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_QUERY_H 