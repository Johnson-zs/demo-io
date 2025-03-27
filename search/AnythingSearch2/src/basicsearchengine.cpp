#include "basicsearchengine.h"
#include <QDir>
#include <QDirIterator>
#include <QtConcurrent/QtConcurrent>

BasicSearchEngine::BasicSearchEngine()
{
}

void BasicSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_allFiles.clear();
    
    // 开始索引文件
    QtConcurrent::run([this]() {
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