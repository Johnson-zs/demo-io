#include "basicsearchengine.h"
#include <QDir>
#include <QDirIterator>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>

BasicSearchEngine::BasicSearchEngine()
{
}

void BasicSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_allFiles.clear();
    
    // 修复警告：保存返回值或使用 QThreadPool
    QThreadPool::globalInstance()->start([this]() {
        indexDirectory(m_currentPath);
    });
}

void BasicSearchEngine::indexDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    
    QDirIterator it(path, QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs,
                    QDirIterator::Subdirectories);
    
    QVector<FileData> newFiles;
    
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        newFiles.append(FileData::fromFileInfo(fileInfo));
    }
    
    m_allFiles = newFiles;
}

QVector<FileData> BasicSearchEngine::getAllFiles(int limit) const
{
    if (limit < 0 || limit >= m_allFiles.size()) {
        return m_allFiles;
    }
    
    return QVector<FileData>(m_allFiles.constBegin(), m_allFiles.constBegin() + limit);
}

QVector<FileData> BasicSearchEngine::searchFiles(const QString &keyword) const
{
    if (keyword.isEmpty())
        return m_allFiles;
    
    QVector<FileData> results;
    
    QString lowerKeyword = keyword.toLower();
    
    for (const FileData &file : m_allFiles) {
        if (file.name.toLower().contains(lowerKeyword) ||
            file.path.toLower().contains(lowerKeyword)) {
            results.append(file);
        }
    }
    
    return results;
}

void BasicSearchEngine::cancelSearch()
{
    // 基础搜索引擎不需要特殊处理
}

void BasicSearchEngine::clearCache()
{
    m_allFiles.clear();
}

QVector<FileData> BasicSearchEngine::searchFilesBatch(const QString &keyword, int offset, int limit) const
{
    // 先获取所有匹配结果
    QVector<FileData> allResults = searchFiles(keyword);
    
    // 计算分页
    int startIdx = qMin(offset, allResults.size());
    int endIdx = qMin(offset + limit, allResults.size());
    
    // 返回指定范围的结果
    if (startIdx >= endIdx) {
        return QVector<FileData>();
    }
    
    return QVector<FileData>(allResults.begin() + startIdx, allResults.begin() + endIdx);
}

int BasicSearchEngine::getSearchResultCount(const QString &keyword) const
{
    return searchFiles(keyword).size();
} 