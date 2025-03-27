#include "basicsearchengine.h"
#include <QDir>
#include <QDirIterator>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include <QFileInfo>
#include <QDebug>

BasicSearchEngine::BasicSearchEngine()
{
}

void BasicSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_cachedFiles.clear();
}

QVector<FileData> BasicSearchEngine::getAllFiles(int limit) const
{
    if (m_cachedFiles.isEmpty()) {
        QStringList allFiles = scanDirectory(m_currentPath);
        
        m_cachedFiles.reserve(allFiles.size());
        for (const QString &filePath : allFiles) {
            QFileInfo fileInfo(filePath);
            if (fileInfo.exists()) {
                m_cachedFiles.append(FileData::fromFileInfo(fileInfo));
            }
        }
    }
    
    if (limit < 0 || limit >= m_cachedFiles.size()) {
        return m_cachedFiles;
    }
    
    return QVector<FileData>(m_cachedFiles.constBegin(), m_cachedFiles.constBegin() + limit);
}

QVector<FileData> BasicSearchEngine::searchFiles(const QString &keyword, 
                                               bool caseSensitive,
                                               bool fuzzySearch) const
{
    m_searchCancelled = false;
    
    if (keyword.isEmpty()) {
        return getAllFiles();
    }
    
    return performSearch(keyword, caseSensitive, fuzzySearch);
}

QVector<FileData> BasicSearchEngine::performSearch(const QString &keyword, 
                                                 bool caseSensitive,
                                                 bool fuzzySearch) const
{
    // 确保有缓存
    if (m_cachedFiles.isEmpty()) {
        getAllFiles();
    }
    
    QVector<FileData> results;
    
    // 准备关键词匹配
    Qt::CaseSensitivity caseSens = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QStringList keywords = keyword.split(' ', Qt::SkipEmptyParts);
    
    for (const FileData &file : m_cachedFiles) {
        // 检查是否取消搜索
        if (m_searchCancelled) {
            return QVector<FileData>();
        }
        
        bool match = true;
        for (const QString &kw : keywords) {
            if (fuzzySearch) {
                // 简单的模糊搜索实现：检查是否包含关键词的一部分（至少3个字符）
                // 真正好的实现应该用编辑距离算法，但这里做简化处理
                if (kw.length() >= 3) {
                    // 检查是否包含关键词的部分子串
                    bool partialMatch = false;
                    for (int i = 0; i <= kw.length() - 3; ++i) {
                        QString part = kw.mid(i, 3);
                        if (file.name.contains(part, caseSens)) {
                            partialMatch = true;
                            break;
                        }
                    }
                    if (!partialMatch) {
                        match = false;
                        break;
                    }
                } else {
                    // 如果关键词太短，就用普通匹配
                    if (!file.name.contains(kw, caseSens)) {
                        match = false;
                        break;
                    }
                }
            } else {
                // 普通搜索
                if (!file.name.contains(kw, caseSens)) {
                    match = false;
                    break;
                }
            }
        }
        
        if (match) {
            results.append(file);
        }
    }
    
    return results;
}

QStringList BasicSearchEngine::scanDirectory(const QString &path) const
{
    QStringList result;
    QDirIterator it(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        it.next();
        result.append(it.filePath());
    }
    
    return result;
}

void BasicSearchEngine::cancelSearch()
{
    m_searchCancelled = true;
}

void BasicSearchEngine::clearCache()
{
    m_cachedFiles.clear();
}

QVector<FileData> BasicSearchEngine::searchFilesBatch(const QString &keyword, 
                                                    int offset, 
                                                    int limit,
                                                    bool caseSensitive,
                                                    bool fuzzySearch) const
{
    // 先获取全部搜索结果
    QVector<FileData> allResults = performSearch(keyword, caseSensitive, fuzzySearch);
    
    // 计算分页
    int startIdx = qMin(offset, allResults.size());
    int endIdx = qMin(offset + limit, allResults.size());
    
    if (startIdx >= endIdx) {
        return QVector<FileData>();
    }
    
    return QVector<FileData>(allResults.constBegin() + startIdx, allResults.constBegin() + endIdx);
}

int BasicSearchEngine::getSearchResultCount(const QString &keyword,
                                          bool caseSensitive,
                                          bool fuzzySearch) const
{
    return performSearch(keyword, caseSensitive, fuzzySearch).size();
} 