#include "searchmanager.h"
#include "basicsearchengine.h"
#include "lucenesearchengine.h"
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

SearchManager::SearchManager(QObject *parent)
    : QObject(parent)
{
    // 默认使用Lucene搜索引擎，因为已经添加了依赖
    m_searchEngine = std::make_unique<LuceneSearchEngine>();
    
    connect(&m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &SearchManager::onDirectoryChanged);
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

QVector<FileData> SearchManager::searchFiles(const QString &keyword) const
{
    return m_searchEngine->searchFiles(keyword);
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