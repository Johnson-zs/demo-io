#include "filewatcher.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>

#include "utils/logger.h"

// VFS事件类型常量定义
// 与内核模块保持一致
#define ACT_NEW_FILE 0
#define ACT_NEW_LINK 1
#define ACT_NEW_SYMLINK 2
#define ACT_NEW_FOLDER 3
#define ACT_DEL_FILE 4
#define ACT_DEL_FOLDER 5
#define ACT_RENAME_FROM_FILE 6
#define ACT_RENAME_FROM_FOLDER 7
#define ACT_RENAME_TO_FILE 8
#define ACT_RENAME_TO_FOLDER 9
#define ACT_RENAME_FILE 10
#define ACT_RENAME_FOLDER 11
#define ACT_MOUNT 12
#define ACT_UNMOUNT 13
#define ACT_CLOSE_WRITE_FILE	14
#define ACT_CLOSE_NOWRITE_FILE	15

FileWatcher::FileWatcher(QObject* parent)
    : QObject(parent)
    , eventListener_(new EventListener(this))
    , watchPath_(QDir::homePath()) {
    
    connect(eventListener_, &EventListener::eventReceived,
            this, &FileWatcher::handleEvent);
            
    // 初始化重命名事件缓存
    renameFromEvents_.clear();
}

FileWatcher::~FileWatcher() {
    stopWatching();
}

bool FileWatcher::startWatching() {
    Logger::logInfo(QString("Starting to watch directory: %1").arg(watchPath_));
    
    // 使用异步方式启动监听
    eventListener_->asyncListen();
    return true;
}

void FileWatcher::stopWatching() {
    Logger::logInfo("Stopping file watcher");
    eventListener_->stopListening();
}

void FileWatcher::setWatchPath(const QString& path) {
    watchPath_ = path;
    Logger::logInfo(QString("Watch path set to: %1").arg(watchPath_));
}

QString FileWatcher::watchPath() const {
    return watchPath_;
}

void FileWatcher::handleEvent(const FSEvent& event) {
    // 首先检查是否是挂载/卸载事件
    if (event.type == ACT_MOUNT || event.type == ACT_UNMOUNT) {
        Logger::logInfo(QString("%1: %2")
            .arg(event.type == ACT_MOUNT ? "Mount device" : "Unmount device")
            .arg(event.path));
        // 可以在这里更新挂载点信息
        return;
    }
    
    // 只处理HOME目录下的事件
    if (!isInWatchPath(event.path)) {
        return;
    }
    
    // 忽略隐藏文件
    if (event.path.contains("/.")) {
        return;
    }
    
    // 处理重命名事件
    if (event.type == ACT_RENAME_FROM_FILE || event.type == ACT_RENAME_FROM_FOLDER) {
        // 保存重命名前的路径信息
        renameFromEvents_[event.cookie] = event.path;
        return;
    } else if (event.type == ACT_RENAME_TO_FILE || event.type == ACT_RENAME_TO_FOLDER) {
        // 处理重命名完成事件
        if (renameFromEvents_.contains(event.cookie)) {
            QString srcPath = renameFromEvents_[event.cookie];
            renameFromEvents_.remove(event.cookie);
            
            // 生成重命名事件
            emit fileRenamed(srcPath, event.path);
            
            // 根据类型发送更具体的事件
            if (event.type == ACT_RENAME_TO_FILE) {
                emit fileModified(event.path);
            } else {
                emit directoryRenamed(srcPath, event.path);
            }
            
            return;
        }
    }
    
    // Log the event
    Logger::logEvent(event);
    
    // Emit the raw event
    emit fileEvent(event);
    
    // Emit specific signals based on event type
    switch (event.type) {
    case ACT_NEW_FILE:
    case ACT_NEW_SYMLINK:
    case ACT_NEW_LINK:
        emit fileCreated(event.path);
        break;
    case ACT_NEW_FOLDER:
        emit directoryCreated(event.path);
        break;
    case ACT_DEL_FILE:
        emit fileDeleted(event.path);
        break;
    case ACT_DEL_FOLDER:
        emit directoryDeleted(event.path);
        break;
    case ACT_RENAME_FILE:
        // 正常情况下不应该直接接收到这种事件类型
        emit fileModified(event.path);
        break;
    case ACT_RENAME_FOLDER:
        // 正常情况下不应该直接接收到这种事件类型
        Logger::logInfo(QString("Directory renamed: %1").arg(event.path));
        break;
    default:
        // Unknown event type, just log
        Logger::logInfo(QString("Unhandled event type: %1, path: %2").arg(event.type).arg(event.path));
        break;
    }
}

bool FileWatcher::isInWatchPath(const QString& path) const {
    // 确保路径是以watchPath_开头的绝对路径
    return path.startsWith(watchPath_);
} 