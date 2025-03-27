#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QObject>
#include <QVector>
#include <QFileSystemWatcher>
#include <memory>
#include "filedata.h"
#include "searchengine.h"

class SearchManager : public QObject
{
    Q_OBJECT

public:
    enum SearchEngineType {
        Basic,
        Lucene
    };

    explicit SearchManager(QObject *parent = nullptr);
    
    // 设置搜索引擎类型
    void setSearchEngineType(SearchEngineType type);
    
    // 更新搜索路径
    void updateSearchPath(const QString &path);
    
    // 获取所有文件（数量限制）
    QVector<FileData> getAllFiles(int limit = -1) const;
    
    // 搜索文件
    QVector<FileData> searchFiles(const QString &keyword) const;

private slots:
    void onDirectoryChanged(const QString &path);
    void reindexFiles();

private:
    QString m_currentPath;
    QFileSystemWatcher m_fileWatcher;
    std::unique_ptr<ISearchEngine> m_searchEngine;
};

#endif // SEARCHMANAGER_H 