#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include "indexconfig.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 索引监控器 - 监控文件系统变更，触发索引更新
class QTSEARCHKIT_EXPORT IndexMonitor : public QObject {
    Q_OBJECT
public:
    explicit IndexMonitor(QObject* parent = nullptr);
    ~IndexMonitor() override;
    
    // 设置监控配置
    void setConfig(const IndexConfig& config);
    IndexConfig config() const;
    
    // 控制监控
    bool startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    
    // 配置节流策略
    void setUpdateThrottleInterval(int msec);
    int updateThrottleInterval() const;
    
    // 最大同时监控文件数
    void setMaxWatchedFiles(int max);
    int maxWatchedFiles() const;
    
signals:
    // 监控事件
    void fileAdded(const QString& path);
    void fileChanged(const QString& path);
    void fileRemoved(const QString& path);
    void directoryChanged(const QString& path);
    
    // 索引相关信号
    void updateNeeded(const QStringList& changedPaths);
    void monitoringStarted();
    void monitoringStopped();
    void monitoringError(const QString& error);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 