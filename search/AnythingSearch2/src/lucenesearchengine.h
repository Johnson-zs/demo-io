#ifndef LUCENESEARCHENGINE_H
#define LUCENESEARCHENGINE_H

#include "searchengine.h"
#include <lucene++/LuceneHeaders.h>
#include <QVector>
#include <atomic>

using namespace Lucene;

class LuceneSearchEngine : public ISearchEngine
{
public:
    explicit LuceneSearchEngine();
    ~LuceneSearchEngine() override = default;
    
    void updateSearchPath(const QString &path) override;
    QVector<FileData> searchFiles(const QString &keyword, 
                                bool caseSensitive = false,
                                bool fuzzySearch = false) const override;
    QVector<FileData> getAllFiles(int limit = -1) const override;
    void cancelSearch() override;
    void clearCache() override;
    QVector<FileData> searchFilesBatch(const QString &keyword, 
                                     int offset, 
                                     int limit,
                                     bool caseSensitive = false,
                                     bool fuzzySearch = false) const override;
    int getSearchResultCount(const QString &keyword,
                            bool caseSensitive = false, 
                            bool fuzzySearch = false) const override;
    
    // 预热索引以提高首次搜索性能
    void warmupIndex() const;
    
private:
    QString getIndexDirectory() const;
    QString getHomeDirectory() const;
    QStringList performLuceneSearch(const QString &path, 
                                   const QString &keyword, 
                                   bool nrt = false,
                                   bool caseSensitive = false,
                                   bool fuzzySearch = false) const;
    QVector<FileData> convertToFileData(const QStringList &paths) const;
    
    // 构建搜索查询
    Lucene::QueryPtr buildSearchQuery(const QString &keyword, 
                                    bool caseSensitive = false,
                                    bool fuzzySearch = false) const;
    
    // 搜索类型枚举
    enum class SearchType {
        Simple,      // 简单搜索
        Wildcard,    // 通配符搜索
        Boolean,     // 布尔搜索
        Fuzzy        // 模糊搜索
    };
    
    // 确定搜索类型
    SearchType determineSearchType(const QString &keyword, bool fuzzySearch) const;
    
    QString m_currentPath;
    mutable QVector<FileData> m_cachedAllFiles;
    mutable std::atomic<bool> m_searchCancelled;
    
    // 添加缓存最近的搜索结果
    mutable QString m_lastKeyword;
    mutable QStringList m_lastSearchResults;
    mutable bool m_lastCaseSensitive;
    mutable bool m_lastFuzzySearch;
};

#endif // LUCENESEARCHENGINE_H 