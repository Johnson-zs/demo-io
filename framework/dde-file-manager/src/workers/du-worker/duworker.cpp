#include "duworker.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace DFM {

// 从core库导入命令常量
namespace Commands {
    // 相同定义...
}

// 插件创建函数实现
Q_DECL_EXPORT WorkerPlugin* createWorkerPlugin()
{
    return new DiskUsageWorker();
}

DiskUsageWorker::DiskUsageWorker(QObject *parent)
    : WorkerPlugin(parent)
{
}

DiskUsageWorker::~DiskUsageWorker()
{
    shutdown();
}

bool DiskUsageWorker::initialize()
{
    qDebug() << "DiskUsage worker initialized";
    return true;
}

void DiskUsageWorker::shutdown()
{
    m_canceled = true;
    qDebug() << "DiskUsage worker shutdown";
}

WorkerPlugin::Result DiskUsageWorker::executeCommand(const QString &command, const QUrl &url, 
                                                  const QVariantMap &args)
{
    m_canceled = false;
    
    // 根据命令分发到不同的处理方法
    if (command == "getDiskUsage") {
        return handleGetDiskUsage(url, args);
    } else if (command == "stat") {
        return handleStatFile(url, args);
    }
    
    // 不支持的命令
    return { Status::NotSupported, QVariant(), QString("Command not supported: %1").arg(command) };
}

WorkerPlugin::Result DiskUsageWorker::handleGetDiskUsage(const QUrl &url, const QVariantMap &args)
{
    // 获取参数
    bool recursive = args.value("recursive", true).toBool();
    
    // 获取本地路径
    QString path = url.toLocalFile();
    if (path.isEmpty()) {
        return { Status::Error, QVariant(), "Invalid URL" };
    }
    
    // 计算磁盘使用情况
    QJsonObject result = calculateDiskUsage(path, recursive);
    
    // 检查是否被取消
    if (m_canceled) {
        return { Status::Error, QVariant(), "Operation canceled" };
    }
    
    // 返回结果
    return { Status::Ok, result };
}

WorkerPlugin::Result DiskUsageWorker::handleStatFile(const QUrl &url, const QVariantMap &args)
{
    QString path = url.toLocalFile();
    if (path.isEmpty()) {
        return { Status::Error, QVariant(), "Invalid URL" };
    }
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return { Status::NotFound, QVariant(), QString("File not found: %1").arg(path) };
    }
    
    QJsonObject result;
    result["name"] = fileInfo.fileName();
    result["isDir"] = fileInfo.isDir();
    result["isFile"] = fileInfo.isFile();
    result["size"] = fileInfo.size();
    result["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
    result["permissions"] = static_cast<int>(fileInfo.permissions());
    
    return { Status::Ok, result };
}

QJsonObject DiskUsageWorker::calculateDiskUsage(const QString &path, bool recursive)
{
    QJsonObject result;
    result["path"] = path;
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        result["exists"] = false;
        return result;
    }
    
    result["exists"] = true;
    
    if (fileInfo.isFile()) {
        result["type"] = "file";
        result["size"] = fileInfo.size();
        result["fileCount"] = 1;
        result["dirCount"] = 0;
        return result;
    }
    
    if (fileInfo.isDir()) {
        result["type"] = "directory";
        
        qint64 totalSize = 0;
        int fileCount = 0;
        int dirCount = 1;  // 包括自身
        
        QDir dir(path);
        QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        int total = entries.size();
        int i = 0;
        
        foreach (const QFileInfo &entry, entries) {
            // 检查是否取消
            if (m_canceled) {
                result["complete"] = false;
                break;
            }
            
            if (entry.isDir() && recursive) {
                QJsonObject subResult = calculateDiskUsage(entry.absoluteFilePath(), recursive);
                totalSize += subResult["size"].toDouble();
                fileCount += subResult["fileCount"].toInt();
                dirCount += subResult["dirCount"].toInt();
                
                if (!subResult["complete"].toBool()) {
                    result["complete"] = false;
                    break;
                }
            } else if (entry.isFile()) {
                totalSize += entry.size();
                fileCount++;
            }
            
            // 报告进度
            i++;
            if (total > 0) {
                int progress = (i * 100) / total;
                emit progressChanged(QUrl::fromLocalFile(path), i, total);
            }
        }
        
        result["size"] = totalSize;
        result["fileCount"] = fileCount;
        result["dirCount"] = dirCount;
        
        if (!result.contains("complete")) {
            result["complete"] = true;
        }
        
        return result;
    }
    
    // 其他类型的文件
    result["type"] = "other";
    result["size"] = fileInfo.size();
    result["fileCount"] = 1;
    result["dirCount"] = 0;
    
    return result;
}

void DiskUsageWorker::cancelCurrentOperation()
{
    m_canceled = true;
    emit operationCanceled(QUrl());
}

} // namespace DFM 