#ifndef LUCENESEARCHENGINE_H
#define LUCENESEARCHENGINE_H

#include "searchengine.h"
#include <lucene++/LuceneHeaders.h>
#include <QVector>

class LuceneSearchEngine : public ISearchEngine
{
public:
    explicit LuceneSearchEngine();
    ~LuceneSearchEngine() override = default;
    
    void updateSearchPath(const QString &path) override;
    QVector<FileData> searchFiles(const QString &keyword) const override;
    QVector<FileData> getAllFiles(int limit = -1) const override;
    
private:
    QString getIndexDirectory() const;
    QString getHomeDirectory() const;
    QStringList performLuceneSearch(const QString &path, const QString &keyword, bool nrt = false) const;
    QVector<FileData> convertToFileData(const QStringList &paths) const;
    
    QString m_currentPath;
    mutable QVector<FileData> m_cachedAllFiles;
};

#endif // LUCENESEARCHENGINE_H 