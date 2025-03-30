#include <dfm-search/plugin_system.h>

#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

namespace DFM {
namespace Search {

// PluginInfo实现
QString PluginInfo::pluginDirectory() const {
    return QFileInfo(filePath).absolutePath();
}

// 插件管理器私有实现
class PluginManager::Impl {
public:
    Impl(PluginManager* q)
        : q_ptr(q)
        , initialized(false)
    {
    }
    
    PluginManager* q_ptr;
    bool initialized;
    QStringList pluginPaths;
    QMap<QString, PluginInfo> loadedPlugins;
    mutable QMutex mutex;
    
    // 根据 ID 获取插件信息
    PluginInfo getPluginInfo(const QString& pluginId) const {
        if (loadedPlugins.contains(pluginId)) {
            return loadedPlugins[pluginId];
        }
        return PluginInfo();
    }
    
    // 扫描插件目录
    void scanPluginDirectories() {
        loadedPlugins.clear();
        
        for (const QString& path : pluginPaths) {
            QDir dir(path);
            if (!dir.exists()) {
                qWarning() << "插件目录不存在:" << path;
                continue;
            }
            
            // 获取所有库文件
            QStringList filters;
#ifdef Q_OS_WIN
            filters << "*.dll";
#else
            filters << "*.so";
#endif
            dir.setNameFilters(filters);
            
            // 尝试加载每个文件
            for (const QString& fileName : dir.entryList(QDir::Files)) {
                QString filePath = dir.absoluteFilePath(fileName);
                tryLoadPlugin(filePath);
            }
        }
    }
    
    // 尝试加载插件
    bool tryLoadPlugin(const QString& filePath) {
        QPluginLoader loader(filePath);
        QObject* plugin = loader.instance();
        
        if (!plugin) {
            qWarning() << "加载插件失败:" << filePath << loader.errorString();
            return false;
        }
        
        // 尝试转换为我们的插件接口
        SearchPlugin* searchPlugin = qobject_cast<SearchPlugin*>(plugin);
        if (!searchPlugin) {
            qWarning() << "插件不是SearchPlugin接口类型:" << filePath;
            loader.unload();
            return false;
        }
        
        // 获取插件信息
        PluginInfo info;
        info.filePath = filePath;
        info.id = searchPlugin->pluginId();
        info.name = searchPlugin->pluginName();
        info.version = searchPlugin->pluginVersion();
        info.description = searchPlugin->pluginDescription();
        info.author = searchPlugin->pluginAuthor();
        info.interfaceVersion = searchPlugin->interfaceVersion();
        info.capabilities = searchPlugin->capabilities();
        info.metadata = searchPlugin->metadata();
        info.instance = searchPlugin;
        
        // 添加到已加载插件列表
        loadedPlugins[info.id] = info;
        
        qInfo() << "成功加载插件:" << info.name << "(" << info.id << ")" << "版本:" << info.version;
        
        return true;
    }
};

// PluginManager实现
PluginManager::PluginManager()
    : d(new Impl(this))
{
}

PluginManager::~PluginManager() = default;

bool PluginManager::initialize(const QStringList& pluginPaths)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        qWarning() << "插件管理器已经初始化";
        return true;
    }
    
    d->pluginPaths = pluginPaths;
    
    // 扫描插件目录
    d->scanPluginDirectories();
    
    d->initialized = true;
    
    emit initialized();
    
    return true;
}

QStringList PluginManager::getPluginIds() const
{
    QMutexLocker locker(&d->mutex);
    return d->loadedPlugins.keys();
}

PluginInfo PluginManager::getPluginInfo(const QString& pluginId) const
{
    QMutexLocker locker(&d->mutex);
    return d->getPluginInfo(pluginId);
}

QList<PluginInfo> PluginManager::getAllPlugins() const
{
    QMutexLocker locker(&d->mutex);
    return d->loadedPlugins.values();
}

QList<PluginInfo> PluginManager::getPluginsByCapability(PluginCapability capability) const
{
    QMutexLocker locker(&d->mutex);
    
    QList<PluginInfo> result;
    for (const auto& info : d->loadedPlugins.values()) {
        if (info.capabilities & capability) {
            result.append(info);
        }
    }
    
    return result;
}

SearchPlugin* PluginManager::getPlugin(const QString& pluginId) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->loadedPlugins.contains(pluginId)) {
        return nullptr;
    }
    
    return d->loadedPlugins[pluginId].instance;
}

bool PluginManager::loadPlugin(const QString& filePath)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "插件管理器未初始化";
        return false;
    }
    
    bool success = d->tryLoadPlugin(filePath);
    
    if (success) {
        emit pluginLoaded(QFileInfo(filePath).fileName());
    }
    
    return success;
}

bool PluginManager::unloadPlugin(const QString& pluginId)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "插件管理器未初始化";
        return false;
    }
    
    if (!d->loadedPlugins.contains(pluginId)) {
        qWarning() << "插件未加载:" << pluginId;
        return false;
    }
    
    QString filePath = d->loadedPlugins[pluginId].filePath;
    
    // 卸载插件
    QPluginLoader loader(filePath);
    if (!loader.unload()) {
        qWarning() << "无法卸载插件:" << pluginId << loader.errorString();
        return false;
    }
    
    d->loadedPlugins.remove(pluginId);
    
    emit pluginUnloaded(pluginId);
    
    return true;
}

bool PluginManager::reloadPlugin(const QString& pluginId)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "插件管理器未初始化";
        return false;
    }
    
    if (!d->loadedPlugins.contains(pluginId)) {
        qWarning() << "插件未加载:" << pluginId;
        return false;
    }
    
    QString filePath = d->loadedPlugins[pluginId].filePath;
    
    // 卸载并重新加载
    if (unloadPlugin(pluginId)) {
        return loadPlugin(filePath);
    }
    
    return false;
}

void PluginManager::reloadAllPlugins()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "插件管理器未初始化";
        return;
    }
    
    // 保存当前所有插件路径
    QStringList paths;
    for (const auto& info : d->loadedPlugins.values()) {
        paths.append(info.filePath);
    }
    
    // 清空当前插件
    d->loadedPlugins.clear();
    
    // 重新加载
    for (const QString& path : paths) {
        d->tryLoadPlugin(path);
    }
    
    emit pluginsReloaded();
}

// SearchPlugin实现
SearchPlugin::~SearchPlugin() = default;

} // namespace Search
} // namespace DFM 