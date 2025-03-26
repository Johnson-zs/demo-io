#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include "searchquery.h"
#include "searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchCacheEntry {
public:
    SearchCacheEntry();
    
    std::shared_ptr<SearchResultSet> results() const;
    void setResults(std::shared_ptr<SearchResultSet> results);
    
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime& timestamp);
    
    bool isExpired(int maxAgeInSeconds) const;
    
private:
    std::shared_ptr<SearchResultSet> m_results;
    QDateTime m_timestamp;
};

class QTSEARCHKIT_EXPORT SearchCache : public QObject {
    Q_OBJECT
public:
    explicit SearchCache(QObject* parent = nullptr);
    
    // 缓存配置
    void setMaxEntries(int count);
    int maxEntries() const;
    
    void setMaxAgeSeconds(int seconds);
    int maxAgeSeconds() const;
    
    // 缓存操作
    void store(const SearchQuery& query, SearchType type, std::shared_ptr<SearchResultSet> results);
    std::shared_ptr<SearchResultSet> retrieve(const SearchQuery& query, SearchType type);
    bool contains(const SearchQuery& query, SearchType type) const;
    
    void clear();
    void removeExpiredEntries();
    
    // 缓存统计
    int entryCount() const;
    int hitCount() const;
    int missCount() const;
    
private:
    QString generateCacheKey(const SearchQuery& query, SearchType type) const;
    
    QMap<QString, SearchCacheEntry> m_cache;
    int m_maxEntries = 100;
    int m_maxAgeSeconds = 300; // 5分钟
    int m_hits = 0;
    int m_misses = 0;
};

} // namespace QtSearchKit 