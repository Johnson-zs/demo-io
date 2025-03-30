#pragma once

#include <QObject>
#include <QHash>
#include <QMap>
#include <QUrl>
#include <memory>
#include "workerplugin.h"

namespace DFM {

class WorkerManager : public QObject
{
    Q_OBJECT
public:
    static WorkerManager *instance();
    ~WorkerManager();
    
    // 加载和管理插件
    bool loadPlugins();
    QStringList availableWorkers() const;
    
    // 获取插件接口
    WorkerPlugin *getWorkerForUrl(const QUrl &url);
    WorkerPlugin *getWorkerByScheme(const QString &scheme);
    WorkerPlugin *getWorkerByName(const QString &name);
    
    // 异步操作
    QFuture<WorkerPlugin::Result> executeAsync(
        WorkerPlugin *worker,
        const QString &operation,
        const QUrl &url,
        const QVariantMap &data = QVariantMap()
    );
    
signals:
    void pluginLoaded(const QString &name);
    void pluginLoadFailed(const QString &path, const QString &error);
    
private:
    explicit WorkerManager(QObject *parent = nullptr);
    void scanPluginDirectory(const QString &path);
    
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 