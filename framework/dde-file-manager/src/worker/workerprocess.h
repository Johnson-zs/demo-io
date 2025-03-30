#pragma once

#include <QObject>
#include <QPluginLoader>
#include <QLocalSocket>
#include <memory>

namespace DFM {

class WorkerPlugin;

// Worker进程类 - 负责加载和管理插件
class WorkerProcess : public QObject
{
    Q_OBJECT
public:
    explicit WorkerProcess(QObject *parent = nullptr);
    ~WorkerProcess();
    
    // 初始化和运行
    bool init(const QString &pluginPath, const QString &connectionName);
    int exec();
    
private slots:
    // 通信相关
    void handleIncomingCommand(const QByteArray &data);
    void handleConnectionError();
    void handleDisconnected();
    
    // 插件进度回调
    void onPluginProgress(const QUrl &url, qint64 processed, qint64 total);
    void onOperationFinished(const QUrl &url, const WorkerPlugin::Result &result);
    void onOperationCanceled(const QUrl &url);
    
private:
    // 辅助方法
    bool loadPlugin(const QString &path);
    void sendResponse(const QByteArray &data);
    
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 