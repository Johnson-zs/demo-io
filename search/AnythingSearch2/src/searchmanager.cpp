#include "searchmanager.h"
#include "basicsearchengine.h"
#include "lucenesearchengine.h"
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

SearchManager::SearchManager(QObject *parent)
    : QObject(parent)
    , m_currentStatus(Idle)
    , m_lastCaseSensitive(false)
    , m_lastFuzzySearch(false)
{
    // 默认使用Lucene搜索引擎，因为已经添加了依赖
    m_searchEngine = std::make_unique<LuceneSearchEngine>();
    
    connect(&m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &SearchManager::onDirectoryChanged);
    
    connect(&m_searchWatcher, &QFutureWatcher<QVector<FileData>>::finished,
            this, [this]() {
                QVector<FileData> results = m_searchFuture.result();
                QString statusMessage = QString("找到 %1 个文件").arg(results.size());
                emit searchStatusChanged(Completed, statusMessage);
                emit searchResultsReady(results);
            });
}

void SearchManager::setSearchEngineType(SearchEngineType type)
{
    // 保存当前路径
    QString currentPath = m_currentPath;
    
    // 根据类型创建相应的搜索引擎
    switch (type) {
    case Basic:
        m_searchEngine = std::make_unique<BasicSearchEngine>();
        break;
    case Lucene:
        m_searchEngine = std::make_unique<LuceneSearchEngine>();
        break;
    }
    
    // 如果已有搜索路径，则重新设置
    if (!currentPath.isEmpty()) {
        updateSearchPath(currentPath);
    }
}

void SearchManager::updateSearchPath(const QString &path)
{
    // 移除旧路径的监视
    if (!m_currentPath.isEmpty() && m_fileWatcher.directories().contains(m_currentPath)) {
        m_fileWatcher.removePath(m_currentPath);
    }
    
    m_currentPath = path;
    
    // 添加新路径的监视
    m_fileWatcher.addPath(m_currentPath);
    
    // 更新搜索引擎路径
    m_searchEngine->updateSearchPath(path);
}

QVector<FileData> SearchManager::getAllFiles(int limit) const
{
    return m_searchEngine->getAllFiles(limit);
}

QVector<FileData> SearchManager::searchFiles(const QString &keyword, 
                                           bool caseSensitive,
                                           bool fuzzySearch) const
{
    if (keyword.isEmpty()) {
        emit const_cast<SearchManager*>(this)->searchStatusChanged(Idle, "搜索已取消");
        return QVector<FileData>();
    }
    
    emit const_cast<SearchManager*>(this)->searchStatusChanged(Searching, "正在搜索...");
    
    m_lastCaseSensitive = caseSensitive;
    m_lastFuzzySearch = fuzzySearch;
    
    QVector<FileData> results = m_searchEngine->searchFiles(keyword, caseSensitive, fuzzySearch);
    
    QString statusMessage = QString("找到 %1 个文件").arg(results.size());
    emit const_cast<SearchManager*>(this)->searchStatusChanged(Completed, statusMessage);
    
    return results;
}

QVector<FileData> SearchManager::searchFilesBatch(const QString &keyword, 
                                               int offset, 
                                               int limit,
                                               bool caseSensitive,
                                               bool fuzzySearch) const
{
    if (keyword.isEmpty()) {
        return QVector<FileData>();
    }
    
    if (offset == 0) { // 首次搜索
        emit const_cast<SearchManager*>(this)->searchStatusChanged(Searching, "正在搜索...");
        m_lastCaseSensitive = caseSensitive;
        m_lastFuzzySearch = fuzzySearch;
    }
    
    QVector<FileData> results = m_searchEngine->searchFilesBatch(
        keyword, offset, limit, caseSensitive, fuzzySearch);
    
    if (offset == 0) { // 只在首次搜索完成时更新状态
        // 注意：这里直接使用cached结果的数量，避免重复搜索
        int total = m_searchEngine->getSearchResultCount(keyword, caseSensitive, fuzzySearch);
        QString statusMessage = QString("找到 %1 个文件").arg(total);
        emit const_cast<SearchManager*>(this)->searchStatusChanged(Completed, statusMessage);
    }
    
    return results;
}

int SearchManager::getSearchResultCount(const QString &keyword,
                                      bool caseSensitive,
                                      bool fuzzySearch) const
{
    return m_searchEngine->getSearchResultCount(keyword, caseSensitive, fuzzySearch);
}

void SearchManager::onDirectoryChanged(const QString &path)
{
    // 延迟重新索引，避免频繁更新
    QTimer::singleShot(2000, this, &SearchManager::reindexFiles);
}

void SearchManager::reindexFiles()
{
    if (!m_currentPath.isEmpty()) {
        m_searchEngine->updateSearchPath(m_currentPath);
    }
}

void SearchManager::clearSearchResults()
{
    if (m_searchEngine) {
        m_searchEngine->clearCache();
    }
    emit searchStatusChanged(Idle, "");
}

void SearchManager::cancelSearch()
{
    if (m_searchEngine) {
        m_searchEngine->cancelSearch();
    }
    emit searchStatusChanged(Idle, "搜索已取消");
}

void SearchManager::searchFilesAsync(const QString &keyword,
                                   bool caseSensitive,
                                   bool fuzzySearch)
{
    if (keyword.isEmpty()) {
        emit searchStatusChanged(Idle, "搜索已取消");
        emit searchResultsReady(QVector<FileData>());
        return;
    }
    
    emit searchStatusChanged(Searching, "正在搜索...");
    
    // 记录搜索选项
    m_lastCaseSensitive = caseSensitive;
    m_lastFuzzySearch = fuzzySearch;
    
    // 取消之前的搜索
    if (m_searchFuture.isRunning()) {
        m_searchEngine->cancelSearch();
    }
    
    // 启动异步搜索
    m_searchFuture = QtConcurrent::run([this, keyword, caseSensitive, fuzzySearch]() {
        return m_searchEngine->searchFiles(keyword, caseSensitive, fuzzySearch);
    });
    
    m_searchWatcher.setFuture(m_searchFuture);
} 