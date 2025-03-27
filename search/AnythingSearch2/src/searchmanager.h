#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QObject>
#include <QVector>
#include <QFileSystemWatcher>
#include <memory>
#include "filedata.h"
#include "searchengine.h"
#include <QFuture>
#include <QFutureWatcher>

class SearchManager : public QObject
{
    Q_OBJECT

public:
    enum SearchEngineType {
        Basic,
        Lucene
    };

    // 搜索状态枚举
    enum SearchStatus {
        Idle,           // 空闲
        Searching,      // 正在搜索
        Completed,      // 搜索完成
        Error           // 发生错误
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

    // 批量搜索文件方法
    QVector<FileData> searchFilesBatch(const QString &keyword, int offset, int limit) const;
    
    // 获取搜索结果总数
    int getSearchResultCount(const QString &keyword) const;

    // 清空搜索结果和缓存
    void clearSearchResults();
    
    // 取消当前搜索
    void cancelSearch();

    // 异步搜索方法
    void searchFilesAsync(const QString &keyword);

signals:
    // 搜索状态变化信号
    void searchStatusChanged(SearchStatus status, const QString &message = QString());
    // 搜索进度信号
    void searchProgressUpdated(int current, int total);
    
    // 搜索结果就绪信号
    void searchResultsReady(const QVector<FileData> &results);

private slots:
    void onDirectoryChanged(const QString &path);
    void reindexFiles();

private:
    QString m_currentPath;
    QFileSystemWatcher m_fileWatcher;
    std::unique_ptr<ISearchEngine> m_searchEngine;
    SearchStatus m_currentStatus;
    
    // 异步搜索结果
    mutable QFuture<QVector<FileData>> m_searchFuture;
    mutable QFutureWatcher<QVector<FileData>> m_searchWatcher;
};

#endif // SEARCHMANAGER_H 