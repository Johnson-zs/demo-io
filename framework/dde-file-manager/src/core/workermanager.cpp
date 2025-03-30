#include "workermanager.h"
#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFutureInterface>
#include <QtConcurrent>
#include <QDebug>

namespace DFM {

class WorkerManager::Private {
public:
    // 按名称存储加载的插件
    QHash<QString, WorkerPlugin*> workersByName;
    
    // 按协议存储插件
    QHash<QString, WorkerPlugin*> workersByScheme;
    
    // 插件加载器
    QList<QPluginLoader*> loaders;
};

// 单例实现
WorkerManager *WorkerManager::instance()
{
    static WorkerManager instance;
    return &instance;
}

WorkerManager::WorkerManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

WorkerManager::~WorkerManager()
{
    // 关闭所有插件
    for (auto worker : d->workersByName) {
        worker->shutdown();
    }
    
    // 卸载插件
    for (auto loader : d->loaders) {
        loader->unload();
        delete loader;
    }
}

bool WorkerManager::loadPlugins()
{
    bool anyLoaded = false;
    
    // 查找多个可能的插件目录
    QStringList pluginDirs = QStandardPaths::locateAll(
        QStandardPaths::GenericLibraryLocation,
        "dde-file-manager/workers",
        QStandardPaths::LocateDirectory
    );
    
    // 添加应用程序目录
    pluginDirs << QCoreApplication::applicationDirPath() + "/plugins";
    
    // 扫描每个目录
    for (const QString &dirPath : pluginDirs) {
        scanPluginDirectory(dirPath);
    }
    
    return !d->workersByName.isEmpty();
}

void WorkerManager::scanPluginDirectory(const QString &path)
{
    QDir dir(path);
    
    foreach (QString fileName, dir.entryList(QDir::Files)) {
        QString filePath = dir.absoluteFilePath(fileName);
        
        if (!QLibrary::isLibrary(filePath)) {
            continue;
        }
        
        QPluginLoader *loader = new QPluginLoader(filePath);
        QJsonObject metadata = loader->metaData().value("MetaData").toObject();
        
        // 尝试加载插件
        if (!loader->load()) {
            qWarning() << "Failed to load plugin:" << filePath << "-" << loader->errorString();
            emit pluginLoadFailed(filePath, loader->errorString());
            delete loader;
            continue;
        }
        
        // 获取插件实例
        QObject *instance = loader->instance();
        if (!instance) {
            qWarning() << "Failed to get plugin instance:" << filePath;
            emit pluginLoadFailed(filePath, "Failed to get plugin instance");
            loader->unload();
            delete loader;
            continue;
        }
        
        // 尝试将实例转换为 WorkerPlugin
        WorkerPlugin *plugin = qobject_cast<WorkerPlugin*>(instance);
        if (!plugin) {
            qWarning() << "Invalid plugin (not a WorkerPlugin):" << filePath;
            emit pluginLoadFailed(filePath, "Invalid plugin type");
            loader->unload();
            delete loader;
            continue;
        }
        
        // 初始化插件
        if (!plugin->initialize()) {
            qWarning() << "Plugin initialization failed:" << filePath;
            emit pluginLoadFailed(filePath, "Initialization failed");
            loader->unload();
            delete loader;
            continue;
        }
        
        // 保存插件信息
        d->loaders.append(loader);
        d->workersByName.insert(plugin->name(), plugin);
        
        // 注册插件支持的协议
        foreach (const QString &scheme, plugin->supportedSchemes()) {
            d->workersByScheme.insert(scheme, plugin);
        }
        
        emit pluginLoaded(plugin->name());
    }
}

QStringList WorkerManager::availableWorkers() const
{
    return d->workersByName.keys();
}

WorkerPlugin *WorkerManager::getWorkerForUrl(const QUrl &url)
{
    return getWorkerByScheme(url.scheme());
}

WorkerPlugin *WorkerManager::getWorkerByScheme(const QString &scheme)
{
    return d->workersByScheme.value(scheme, nullptr);
}

WorkerPlugin *WorkerManager::getWorkerByName(const QString &name)
{
    return d->workersByName.value(name, nullptr);
}

QFuture<WorkerPlugin::Result> WorkerManager::executeAsync(
    WorkerPlugin *worker,
    const QString &command,
    const QUrl &url,
    const QVariantMap &args)
{
    if (!worker) {
        QFutureInterface<WorkerPlugin::Result> futureInterface;
        futureInterface.reportStarted();
        futureInterface.reportResult({
            WorkerPlugin::Status::Error,
            QVariant(),
            "No worker provided"
        });
        futureInterface.reportFinished();
        return futureInterface.future();
    }
    
    // 创建一个异步任务
    return QtConcurrent::run([worker, command, url, args]() {
        return worker->executeCommand(command, url, args);
    });
}

} // namespace DFM 