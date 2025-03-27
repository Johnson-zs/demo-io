#ifndef BASICSEARCHENGINE_H
#define BASICSEARCHENGINE_H

#include "searchengine.h"
#include <QVector>

class BasicSearchEngine : public ISearchEngine
{
public:
    explicit BasicSearchEngine();
    ~BasicSearchEngine() override = default;
    
    void updateSearchPath(const QString &path) override;
    QVector<FileData> searchFiles(const QString &keyword) const override;
    QVector<FileData> getAllFiles(int limit = -1) const override;
    
private:
    void indexDirectory(const QString &path);
    
    QString m_currentPath;
    QVector<FileData> m_allFiles;
};

#endif // BASICSEARCHENGINE_H 