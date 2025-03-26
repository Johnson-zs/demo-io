#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <functional>
#include "global.h"

namespace QSearch {

enum class QueryType {
    Filename,       // 文件名搜索
    FileContent,    // 文件内容搜索
    Both,           // 文件名和内容
    Extended        // 扩展搜索类型
};

enum class MatchType {
    Exact,          // 精确匹配
    Contains,       // 包含匹配
    StartsWith,     // 开头匹配
    EndsWith,       // 结尾匹配
    Regex,          // 正则表达式匹配
    Fuzzy,          // 模糊匹配
    Pinyin          // 拼音匹配
};

class QSEARCH_EXPORT SearchQuery {
public:
    SearchQuery();
    explicit SearchQuery(const QString& queryText);
    
    // 设置查询类型
    SearchQuery& setType(QueryType type);
    
    // 设置匹配类型
    SearchQuery& setMatchType(MatchType matchType);
    
    // 设置查询文本
    SearchQuery& setText(const QString& text);
    
    // 设置搜索路径
    SearchQuery& setPaths(const QStringList& paths);
    SearchQuery& addPath(const QString& path);
    
    // 文件过滤
    SearchQuery& setFileFilters(const QStringList& filters);
    SearchQuery& addFileFilter(const QString& filter);
    
    // 大小限制
    SearchQuery& setSizeLimit(qint64 minSize, qint64 maxSize);
    
    // 时间限制
    SearchQuery& setTimeLimit(const QDateTime& startTime, const QDateTime& endTime);
    
    // 高级选项设置
    SearchQuery& setOption(const QString& key, const QVariant& value);
    QVariant option(const QString& key) const;
    
    // 构建复杂查询逻辑
    SearchQuery& and_(const SearchQuery& query);
    SearchQuery& or_(const SearchQuery& query);
    SearchQuery& not_(const SearchQuery& query);
    
    // 获取设置的值
    QString text() const;
    QueryType type() const;
    MatchType matchType() const;
    QStringList paths() const;
    QStringList fileFilters() const;
    
    // 其他成员...
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 