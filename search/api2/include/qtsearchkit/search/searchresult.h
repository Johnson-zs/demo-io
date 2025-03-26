#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QDateTime>
#include <memory>
#include "qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchResultItem {
public:
    SearchResultItem();
    explicit SearchResultItem(const QUrl& uri);
    
    // 基本属性
    QUrl uri() const;
    void setUri(const QUrl& uri);
    
    QString title() const;
    void setTitle(const QString& title);
    
    QString description() const;
    void setDescription(const QString& description);
    
    Relevance relevance() const;
    void setRelevance(Relevance relevance);
    
    QDateTime modifiedTime() const;
    void setModifiedTime(const QDateTime& time);
    
    SearchType searchType() const;
    void setSearchType(SearchType type);
    
    // 动态属性扩展
    template<typename T>
    void setMetadata(const QString& key, const T& value) {
        m_metadata.insert(key, QVariant::fromValue(value));
    }
    
    QVariant metadata(const QString& key) const;
    bool hasMetadata(const QString& key) const;
    
private:
    QUrl m_uri;
    QString m_title;
    QString m_description;
    Relevance m_relevance = Relevance::Medium;
    QDateTime m_modifiedTime;
    SearchType m_searchType = SearchType::Custom;
    QVariantMap m_metadata;
};

// 搜索结果集合类
class QTSEARCHKIT_EXPORT SearchResultSet : public QObject {
    Q_OBJECT
public:
    explicit SearchResultSet(QObject* parent = nullptr);
    
    void addItem(const SearchResultItem& item);
    SearchResultItem itemAt(int index) const;
    int count() const;
    bool isEmpty() const;
    
    // 元数据
    int totalAvailable() const;
    void setTotalAvailable(int total);
    
    qint64 searchTimeMs() const;
    void setSearchTimeMs(qint64 timeMs);
    
    bool hasMore() const;
    void setHasMore(bool hasMore);
    
    // 高级功能：可自定义排序
    using SortFunction = std::function<bool(const SearchResultItem&, const SearchResultItem&)>;
    void sort(const SortFunction& sortFunction);
    
    // 基本迭代支持
    QVector<SearchResultItem>::const_iterator begin() const;
    QVector<SearchResultItem>::const_iterator end() const;
    
private:
    QVector<SearchResultItem> m_items;
    int m_totalAvailable = 0;
    qint64 m_searchTimeMs = 0;
    bool m_hasMore = false;
};

} // namespace QtSearchKit 