#ifndef BASICSEARCHENGINE_H
#define BASICSEARCHENGINE_H

#include "searchengine.h"
#include <QDir>
#include <QVector>

class BasicSearchEngine : public ISearchEngine
{
public:
    BasicSearchEngine();
    ~BasicSearchEngine() override = default;
    
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
    
private:
    QString m_currentPath;
    mutable QVector<FileData> m_cachedFiles;
    mutable std::atomic<bool> m_searchCancelled{false};
    
    QVector<FileData> performSearch(const QString &keyword, 
                                   bool caseSensitive,
                                   bool fuzzySearch) const;
    QStringList scanDirectory(const QString &path) const;
};

#endif // BASICSEARCHENGINE_H 