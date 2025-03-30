#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QJsonObject>

namespace DFM {

// 简化的 Worker 插件接口
class WorkerPlugin : public QObject
{
    Q_OBJECT
public:
    // 操作状态枚举
    enum class Status {
        Ok,
        Error,
        NotFound,
        AccessDenied,
        NotSupported,
        Timeout
    };
    
    // 简化的结果结构
    struct Result {
        Status status = Status::Ok;
        QVariant data;
        QString errorMessage;
    };
    
    explicit WorkerPlugin(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~WorkerPlugin() {}
    
    // 基本插件信息
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QStringList supportedSchemes() const = 0;
    
    // 简化的核心方法
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // 核心操作 - 只保留执行命令的通用接口
    virtual Result executeCommand(const QString &command, const QUrl &url, 
                                 const QVariantMap &args = QVariantMap()) = 0;

signals:
    void progressChanged(const QUrl &url, qint64 bytesProcessed, qint64 bytesTotal);
    void operationFinished(const QUrl &url, const Result &result);
    void operationCanceled(const QUrl &url);
};

// 定义插件接口
#define WorkerPlugin_iid "org.deepin.dde.file-manager.WorkerPlugin/1.0"
Q_DECLARE_INTERFACE(DFM::WorkerPlugin, WorkerPlugin_iid)

} // namespace DFM 