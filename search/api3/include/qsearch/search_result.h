#pragma once

#include <QString>
#include <QVector>
#include <QVariant>
#include <QUrl>
#include <QDateTime>
#include <QSharedPointer>
#include "global.h"

namespace QSearch {

struct QSEARCH_EXPORT ResultItem {
    QUrl url;                // 文件URL
    QString name;            // 文件名
    QString path;            // 文件路径
    qint64 size = 0;         // 文件大小
    QDateTime modifiedTime;  // 修改时间
    bool isDirectory = false;// 是否是目录
    
    // 匹配详情
    struct MatchInfo {
        int position;        // 匹配位置
        int length;          // 匹配长度
        QString context;     // 上下文内容（全文搜索时）
    };
    QVector<MatchInfo> matches;
    
    // 扩展数据
    QVariantMap metadata;
};

class QSEARCH_EXPORT SearchResult {
public:
    SearchResult();
    
    // 添加结果项
    void addItem(const ResultItem& item);
    
    // 获取结果集
    QVector<ResultItem> items() const;
    
    // 结果统计
    int count() const;
    bool isEmpty() const;
    
    // 分页支持
    void setPage(int page, int pageSize);
    QVector<ResultItem> currentPage() const;
    bool hasNextPage() const;
    bool hasPreviousPage() const;
    
    // 排序
    enum class SortField { Name, Path, Size, ModifiedTime };
    enum class SortOrder { Ascending, Descending };
    void sort(SortField field, SortOrder order = SortOrder::Ascending);
    
    // 过滤
    SearchResult filtered(const std::function<bool(const ResultItem&)>& predicate) const;
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 