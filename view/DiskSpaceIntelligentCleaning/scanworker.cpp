#include "scanworker.h"

#include <QDebug>

ScanWorker::ScanWorker(QObject *parent)
    : QObject(parent)
{
}

void ScanWorker::startScan(const QString &mountPoint)
{
    m_mountPoint = mountPoint;
    
    // TODO: 实现磁盘扫描逻辑
    
    emit progressUpdated(0, "开始扫描磁盘...");
    
    // 发出扫描完成信号
    emit scanFinished();
}

void ScanWorker::findLargeFiles()
{
    // TODO: 实现查找大文件逻辑 (>50MB)
    
    emit progressUpdated(25, "正在查找大文件...");
    
    // 示例：发出找到的大文件列表
    // emit largeFilesFound(files);
}

void ScanWorker::findDuplicateFiles()
{
    // TODO: 实现查找重复文件逻辑
    
    emit progressUpdated(50, "正在查找重复文件...");
    
    // 示例：发出找到的重复文件
    // emit duplicateFileFound(file, count);
}

void ScanWorker::findAppFiles()
{
    // TODO: 实现查找应用文件逻辑
    
    emit progressUpdated(75, "正在查找应用文件...");
    
    // 示例：发出找到的应用文件
    // emit appFileFound(file, appName);
}