#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#include "filewatcher/filewatcher.h"
#include "utils/logger.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Set application info
    QCoreApplication::setApplicationName("FileMonitor");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // Create file watcher for home directory
    FileWatcher watcher;
    
    // By default, it watches HOME directory
    Logger::logInfo(QString("Monitoring HOME directory: %1").arg(watcher.watchPath()));
    
    // Connect to signals for logging
    QObject::connect(&watcher, &FileWatcher::fileCreated, 
                    [](const QString& path) {
                        Logger::logInfo(QString("File created: %1").arg(path));
                    });
    
    QObject::connect(&watcher, &FileWatcher::fileDeleted, 
                    [](const QString& path) {
                        Logger::logInfo(QString("File deleted: %1").arg(path));
                    });
    
    QObject::connect(&watcher, &FileWatcher::fileModified, 
                    [](const QString& path) {
                        Logger::logInfo(QString("File modified: %1").arg(path));
                    });
    
    // Start watching in a separate thread
    watcher.startWatching();
    
    // Run the application
    return app.exec();
} 