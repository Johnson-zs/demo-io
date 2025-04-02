#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QHash>

#include "eventlistener.h"
#include "utils/fsevents.h"

class FileWatcher : public QObject {
    Q_OBJECT
    
public:
    explicit FileWatcher(QObject* parent = nullptr);
    ~FileWatcher();
    
    // Start watching the home directory
    bool startWatching();
    
    // Stop watching
    void stopWatching();
    
    // Set the target directory (defaults to HOME)
    void setWatchPath(const QString& path);
    
    // Get the currently watched path
    QString watchPath() const;
    
signals:
    // Signals for different file events
    void fileCreated(const QString& path);
    void fileDeleted(const QString& path);
    void fileModified(const QString& path);
    
    // Raw event signal
    void fileEvent(const FSEvent& event);
    
    // 增加新的信号
    void fileRenamed(const QString& oldPath, const QString& newPath);
    void directoryCreated(const QString& path);
    void directoryDeleted(const QString& path);
    void directoryRenamed(const QString& oldPath, const QString& newPath);
    
private slots:
    void handleEvent(const FSEvent& event);
    
private:
    // Helper to check if path is inside watched directory
    bool isInWatchPath(const QString& path) const;
    
private:
    EventListener* eventListener_;
    QString watchPath_;
    QHash<uint32_t, QString> renameFromEvents_; // 用于处理重命名事件
};

#endif // FILEWATCHER_H 