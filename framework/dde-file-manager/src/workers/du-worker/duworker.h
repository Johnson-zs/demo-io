#pragma once

#include <QObject>
#include <QFileInfo>
#include <QDir>
#include "workerplugin.h"

namespace DFM {

class DiskUsageWorker : public WorkerPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WorkerPlugin_iid FILE "duworker.json")
    Q_INTERFACES(DFM::WorkerPlugin)
    
public:
    explicit DiskUsageWorker(QObject *parent = nullptr);
    ~DiskUsageWorker() override;
    
    // 插件信息
    QString name() const override { return "DiskUsage"; }
    QString version() const override { return "1.0"; }
    QStringList supportedSchemes() const override { return {"file"}; }
    
    // 初始化和清理
    bool initialize() override;
    void shutdown() override;
    
    // 命令执行接口
    Result executeCommand(const QString &command, const QUrl &url, 
                        const QVariantMap &args = QVariantMap()) override;
    
private:
    // 命令处理方法
    Result handleGetDiskUsage(const QUrl &url, const QVariantMap &args);
    Result handleStatFile(const QUrl &url, const QVariantMap &args);
    
    // 辅助方法
    QJsonObject calculateDiskUsage(const QString &path, bool recursive);
    void cancelCurrentOperation();
    
    // 取消标志
    bool m_canceled = false;
};

} // namespace DFM 