#include "filewatcher.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>

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
    , watchPath_(QDir::homePath())
    , isOverlayFs_(false) {
    
    connect(eventListener_, &EventListener::eventReceived,
            this, &FileWatcher::handleEvent);
            
    // 初始化重命名事件缓存
    renameFromEvents_.clear();
    
    // 获取overlay信息
    updateOverlayInfo();
}

FileWatcher::~FileWatcher() {
    stopWatching();
}

bool FileWatcher::startWatching() {
    Logger::logInfo(QString("Starting to watch directory: %1").arg(watchPath_));
    Logger::logInfo(QString("Overlay filesystem detected: %1").arg(isOverlayFs_ ? "Yes" : "No"));
    if (isOverlayFs_) {
        Logger::logInfo(QString("Overlay upperdir: %1").arg(overlayUpperDir_));
        Logger::logInfo(QString("Overlay lowerdir: %1").arg(overlayLowerDir_));
    }
    
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
    // 转换overlay路径为实际可见路径
    QString normalizedPath = normalizeOverlayPath(event.path);
    
    // 首先检查是否是挂载/卸载事件
    if (event.type == ACT_MOUNT || event.type == ACT_UNMOUNT) {
        Logger::logInfo(QString("%1: %2")
            .arg(event.type == ACT_MOUNT ? "Mount device" : "Unmount device")
            .arg(normalizedPath));
        // 挂载事件可能会改变overlay配置
        updateOverlayInfo();
        return;
    }
    
    // 只处理HOME目录下的事件
    if (!isInWatchPath(normalizedPath)) {
        return;
    }
    
    // 忽略隐藏文件
    if (normalizedPath.contains("/.")) {
        return;
    }
    
    // 处理重命名事件
    if (event.type == ACT_RENAME_FROM_FILE || event.type == ACT_RENAME_FROM_FOLDER) {
        // 保存重命名前的路径信息
        renameFromEvents_[event.cookie] = normalizedPath;
        return;
    } else if (event.type == ACT_RENAME_TO_FILE || event.type == ACT_RENAME_TO_FOLDER) {
        // 处理重命名完成事件
        if (renameFromEvents_.contains(event.cookie)) {
            QString srcPath = renameFromEvents_[event.cookie];
            renameFromEvents_.remove(event.cookie);
            
            // 生成重命名事件
            emit fileRenamed(srcPath, normalizedPath);
            
            // 根据类型发送更具体的事件
            if (event.type == ACT_RENAME_TO_FILE) {
                emit fileModified(normalizedPath);
            } else {
                emit directoryRenamed(srcPath, normalizedPath);
            }
            
            return;
        }
    }
    
    // 创建一个修改过的FSEvent对象
    FSEvent normalizedEvent = event;
    normalizedEvent.path = normalizedPath;
    
    // Log the event
    Logger::logEvent(normalizedEvent);
    
    // Emit the raw event
    emit fileEvent(normalizedEvent);
    
    // Emit specific signals based on event type
    switch (event.type) {
    case ACT_NEW_FILE:
    case ACT_NEW_SYMLINK:
    case ACT_NEW_LINK:
        emit fileCreated(normalizedPath);
        break;
    case ACT_NEW_FOLDER:
        emit directoryCreated(normalizedPath);
        break;
    case ACT_DEL_FILE:
        emit fileDeleted(normalizedPath);
        break;
    case ACT_DEL_FOLDER:
        emit directoryDeleted(normalizedPath);
        break;
    case ACT_RENAME_FILE:
        emit fileModified(normalizedPath);
        break;
    case ACT_RENAME_FOLDER:
        Logger::logInfo(QString("Directory renamed: %1").arg(normalizedPath));
        break;
    case ACT_CLOSE_WRITE_FILE:
        emit fileModified(normalizedPath);
        break;
    default:
        Logger::logInfo(QString("Unhandled event type: %1, path: %2").arg(event.type).arg(normalizedPath));
        break;
    }
}

bool FileWatcher::isInWatchPath(const QString& path) const {
    // 确保路径是以watchPath_开头的绝对路径
    return path.startsWith(watchPath_);
}

QString FileWatcher::normalizeOverlayPath(const QString& overlayPath) const {
    if (!isOverlayFs_) {
        return overlayPath;
    }
    
    // 添加对简化路径的处理
    if (overlayPath.startsWith("/overlay/disable-system-protect/home/upper/")) {
        QString username = QDir::homePath().section("/", -1);
        QString relativePath = overlayPath.mid(QString("/overlay/disable-system-protect/home/upper/").length());
        
        // 如果路径包含用户名，则直接返回正确的路径
        if (relativePath.startsWith(username + "/")) {
            return QDir::homePath() + relativePath.mid(username.length());
        } else {
            return QDir::homePath() + "/" + relativePath;
        }
    }
    
    // 检查路径是否在upperdir中
    if (!overlayUpperDir_.isEmpty() && overlayPath.startsWith(overlayUpperDir_)) {
        QString relativePath = overlayPath.mid(overlayUpperDir_.length());
        
        // 如果相对路径以斜杠开头，则移除它
        if (relativePath.startsWith("/")) {
            relativePath = relativePath.mid(1);
        }
        
        return QDir::homePath() + "/" + relativePath;
    }
    
    // 也检查是否在lowerdir中
    if (!overlayLowerDir_.isEmpty() && overlayPath.startsWith(overlayLowerDir_)) {
        QString relativePath = overlayPath.mid(overlayLowerDir_.length());
        
        // 如果相对路径以斜杠开头，则移除它
        if (relativePath.startsWith("/")) {
            relativePath = relativePath.mid(1);
        }
        
        return QDir::homePath() + "/" + relativePath;
    }
    
    // 如果不是overlay路径或无法转换，则返回原始路径
    return overlayPath;
}

void FileWatcher::updateOverlayInfo() {
    // 读取/proc/mounts文件获取挂载信息
    QFile mountsFile("/proc/mounts");
    if (!mountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::logError("Failed to open /proc/mounts");
        return;
    }
    
    QTextStream in(&mountsFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("overlay") && line.contains("/home")) {
            isOverlayFs_ = true;
            
            // 解析upperdir和lowerdir
            QRegularExpression upperdirRx("upperdir=([^,]+)");
            QRegularExpression lowerdirRx("lowerdir=([^,]+)");
            
            auto upperdirMatch = upperdirRx.match(line);
            auto lowerdirMatch = lowerdirRx.match(line);
            
            if (upperdirMatch.hasMatch()) {
                overlayUpperDir_ = upperdirMatch.captured(1);
            }
            
            if (lowerdirMatch.hasMatch()) {
                // lowerdir可能有多个，以冒号分隔
                overlayLowerDir_ = lowerdirMatch.captured(1).split(":")[0];
            }
            
            break;
        }
    }
    
    mountsFile.close();
    
    // 如果找不到overlay信息，也可以尝试使用findmnt命令
    if (!isOverlayFs_) {
        QProcess process;
        process.start("findmnt", QStringList() << "-t" << "overlay" << "-o" << "OPTIONS" << "-n" << "/home");
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        
        if (!output.isEmpty()) {
            isOverlayFs_ = true;
            
            QRegularExpression upperdirRx("upperdir=([^,]+)");
            QRegularExpression lowerdirRx("lowerdir=([^,]+)");
            
            auto upperdirMatch = upperdirRx.match(output);
            auto lowerdirMatch = lowerdirRx.match(output);
            
            if (upperdirMatch.hasMatch()) {
                overlayUpperDir_ = upperdirMatch.captured(1);
            }
            
            if (lowerdirMatch.hasMatch()) {
                overlayLowerDir_ = lowerdirMatch.captured(1).split(":")[0];
            }
        }
    }
    
    // 添加对简化路径的检查
    if (!isOverlayFs_) {
        // 检查是否有简化的overlay路径格式
        QDir overlayDir("/overlay/disable-system-protect/home/upper");
        if (overlayDir.exists()) {
            isOverlayFs_ = true;
            overlayUpperDir_ = "/overlay/disable-system-protect/home/upper";
            Logger::logInfo("Detected simplified overlay path: " + overlayUpperDir_);
        }
    }
    
    // 记录当前用户的HOME路径，便于调试
    Logger::logInfo(QString("Current user home directory: %1").arg(QDir::homePath()));
} 