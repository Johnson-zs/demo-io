#pragma once

#include <QString>
#include <QVariant>
#include <QMap>
#include "global.h"

namespace QSearch {

enum class SortOrder {
    NoSort,        // 不排序
    NameAsc,       // 按名称升序
    NameDesc,      // 按名称降序
    SizeAsc,       // 按大小升序
    SizeDesc,      // 按大小降序
    DateAsc,       // 按日期升序
    DateDesc,      // 按日期降序
    RelevanceDesc  // 按相关性降序
};

enum class SearchMode {
    Normal,        // 普通搜索
    Fast,          // 快速搜索（可能结果不完整）
    Deep,          // 深度搜索（更全面但更慢）
    Indexed        // 仅搜索已索引内容
};

class QSEARCH_EXPORT SearchOptions {
public:
    SearchOptions();
    
    // 通用选项
    SearchOptions& setMaxResults(int maxResults);
    SearchOptions& setSortOrder(SortOrder order);
    SearchOptions& setSearchMode(SearchMode mode);
    SearchOptions& setCaseSensitive(bool sensitive);
    
    // 高级选项
    SearchOptions& setThreadCount(int count);
    SearchOptions& setTimeoutMs(int timeout);
    SearchOptions& setLowPriority(bool lowPriority);
    
    // 自定义选项
    SearchOptions& setOption(const QString& key, const QVariant& value);
    QVariant option(const QString& key) const;
    bool hasOption(const QString& key) const;
    
    // 获取设置的值
    int maxResults() const;
    SortOrder sortOrder() const;
    SearchMode searchMode() const;
    bool isCaseSensitive() const;
    int threadCount() const;
    int timeoutMs() const;
    bool isLowPriority() const;
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 