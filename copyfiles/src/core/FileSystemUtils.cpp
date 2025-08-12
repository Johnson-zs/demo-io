#include "FileSystemUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QStorageInfo>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <cstring>

qint64 FileSystemUtils::calculateTotalSize(const QString& path)
{
    QFileInfo info(path);
    
    if (!info.exists()) {
        return -1;
    }
    
    if (info.isFile()) {
        return info.size();
    } else if (info.isDir()) {
        return calculateDirectorySize(path);
    }
    
    return -1;
}

qint64 FileSystemUtils::calculateDirectorySize(const QString& dirPath)
{
    qint64 totalSize = 0;
    QDirIterator it(dirPath, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        
        if (fileInfo.isFile()) {
            totalSize += fileInfo.size();
        }
    }
    
    return totalSize;
}

bool FileSystemUtils::getFileList(const QString& dirPath, QStringList& files)
{
    files.clear();
    
    QDir dir(dirPath);
    if (!dir.exists()) {
        return false;
    }
    
    QDirIterator it(dirPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        files.append(it.next());
    }
    
    return true;
}

QString FileSystemUtils::normalizePath(const QString& path)
{
    QFileInfo info(path);
    return info.absoluteFilePath();
}

bool FileSystemUtils::isPathAccessible(const QString& path)
{
    QFileInfo info(path);
    return info.exists() && info.isReadable();
}

bool FileSystemUtils::ensureDirectoryExists(const QString& path)
{
    QDir dir;
    return dir.mkpath(path);
}

QString FileSystemUtils::getErrorString(int errorCode)
{
    return QString::fromLocal8Bit(std::strerror(errorCode));
}

bool FileSystemUtils::hasEnoughSpace(const QString& destinationPath, qint64 requiredBytes)
{
    QStorageInfo storage(destinationPath);
    
    if (!storage.isValid()) {
        return false;
    }
    
    qint64 availableBytes = storage.bytesAvailable();
    
    // Add 10% safety margin
    qint64 safetyMargin = requiredBytes / 10;
    return availableBytes > (requiredBytes + safetyMargin);
} 