#include "framework/plugin_loader.h"
#include <QDebug>
#include <QDirIterator>

namespace Framework {

PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {
}

PluginLoader::~PluginLoader() {
    unloadAllPlugins();
}

int PluginLoader::loadPlugins(const QDir &directory) {
    if (!directory.exists()) {
        qWarning() << "Plugin directory does not exist:" << directory.absolutePath();
        return 0;
    }

    int count = 0;
    QDirIterator it(directory.absolutePath(), QDir::Files);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (QLibrary::isLibrary(filePath)) {
            if (loadPlugin(filePath)) {
                count++;
            }
        }
    }

    qInfo() << "Loaded" << count << "plugins from" << directory.absolutePath();
    return count;
}

bool PluginLoader::loadPlugin(const QString &filePath) {
    if (QPluginLoader *loader = new QPluginLoader(filePath, this)) {
        qInfo() << "Loading plugin:" << filePath;
        
        if (loader->load()) {
            QObject *instance = loader->instance();
            if (instance) {
                Plugin *plugin = qobject_cast<Plugin*>(instance);
                if (plugin) {
                    QString name = plugin->name();
                    
                    // 如果已经加载了同名插件，先卸载
                    if (m_plugins.contains(name)) {
                        qWarning() << "Plugin with name" << name << "already loaded, replacing";
                        m_pluginLoaders[name]->unload();
                        delete m_pluginLoaders[name];
                        m_pluginLoaders.remove(name);
                        m_plugins.remove(name);
                    }
                    
                    // 初始化插件
                    if (plugin->initialize()) {
                        m_pluginLoaders[name] = loader;
                        m_plugins[name] = plugin;
                        
                        qInfo() << "Plugin loaded:" << name << "(" << plugin->version() << ")";
                        emit pluginLoaded(name);
                        return true;
                    } else {
                        qWarning() << "Failed to initialize plugin:" << name;
                        loader->unload();
                        delete loader;
                        emit pluginLoadFailed(filePath, "Failed to initialize plugin");
                    }
                } else {
                    qWarning() << "Not a valid plugin:" << filePath;
                    loader->unload();
                    delete loader;
                    emit pluginLoadFailed(filePath, "Not a valid plugin interface");
                }
            } else {
                qWarning() << "Failed to get instance:" << loader->errorString();
                delete loader;
                emit pluginLoadFailed(filePath, loader->errorString());
            }
        } else {
            qWarning() << "Failed to load plugin:" << loader->errorString();
            delete loader;
            emit pluginLoadFailed(filePath, loader->errorString());
        }
    }
    
    return false;
}

void PluginLoader::unloadAllPlugins() {
    qInfo() << "Unloading all plugins";
    
    // 先关闭所有插件
    for (auto plugin : m_plugins) {
        plugin->shutdown();
    }
    
    // 然后卸载
    for (auto loader : m_pluginLoaders) {
        loader->unload();
        delete loader;
    }
    
    m_pluginLoaders.clear();
    m_plugins.clear();
}

Plugin* PluginLoader::getPlugin(const QString &name) const {
    return m_plugins.value(name, nullptr);
}

QList<Plugin*> PluginLoader::getAllPlugins() const {
    return m_plugins.values();
}

QList<Plugin*> PluginLoader::getPluginsForTaskType(const QString &taskType) const {
    QList<Plugin*> result;
    
    for (auto plugin : m_plugins) {
        if (plugin->supportsTaskType(taskType)) {
            result.append(plugin);
        }
    }
    
    return result;
}

} // namespace Framework 