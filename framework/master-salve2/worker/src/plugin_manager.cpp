#include "worker/plugin_manager.h"
#include <QDir>
#include <QDebug>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
    , m_pluginLoader(new Framework::PluginLoader(this))
{
    // 连接插件加载器信号
    connect(m_pluginLoader, &Framework::PluginLoader::pluginLoaded,
            [this](const QString &name) {
                qInfo() << "Plugin loaded:" << name;
            });

    connect(m_pluginLoader, &Framework::PluginLoader::pluginLoadFailed,
            [](const QString &path, const QString &errorMessage) {
                qWarning() << "Failed to load plugin" << path << ":" << errorMessage;
            });
}

PluginManager::~PluginManager()
{
    unloadPlugins();
}

bool PluginManager::loadPlugins(const QString &pluginDir)
{
    QDir dir(pluginDir);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << pluginDir;
        return false;
    }

    int count = m_pluginLoader->loadPlugins(dir);
    
    // 连接每个插件的信号
    for (auto plugin : m_pluginLoader->getAllPlugins()) {
        connect(plugin, &Framework::Plugin::taskProgress,
                this, &PluginManager::taskProgress);
                
        connect(plugin, &Framework::Plugin::taskCompleted,
                this, &PluginManager::taskCompleted);
    }
    
    return count > 0;
}

void PluginManager::unloadPlugins()
{
    // 取消所有运行中的任务
    for (auto it = m_taskToPlugin.begin(); it != m_taskToPlugin.end(); ++it) {
        QString taskId = it.key();
        QString pluginName = it.value();
        
        Framework::Plugin *plugin = m_pluginLoader->getPlugin(pluginName);
        if (plugin) {
            plugin->cancelTask(taskId);
        }
    }
    
    m_taskToPlugin.clear();
    m_pluginLoader->unloadAllPlugins();
}

QStringList PluginManager::getPluginNames() const
{
    QStringList result;
    for (auto plugin : m_pluginLoader->getAllPlugins()) {
        result.append(plugin->name());
    }
    return result;
}

QStringList PluginManager::getSupportedTaskTypes() const
{
    QSet<QString> result;
    
    for (auto plugin : m_pluginLoader->getAllPlugins()) {
        // 这里我们假设插件支持的任务类型是通过某种方式存储的
        // 实际中可能需要为Plugin接口添加一个获取支持任务类型的方法
        // 这里简单起见，我们通过supportsTaskType方法检查一些常见类型
        
        if (plugin->supportsTaskType("ImageProcessing")) {
            result.insert("ImageProcessing");
        }
        
        if (plugin->supportsTaskType("Calculator")) {
            result.insert("Calculator");
        }
        
        // 可以添加更多类型检查
    }
    
    return result.values();
}

bool PluginManager::supportsTaskType(const QString &taskType) const
{
    for (auto plugin : m_pluginLoader->getAllPlugins()) {
        if (plugin->supportsTaskType(taskType)) {
            return true;
        }
    }
    
    return false;
}

bool PluginManager::executeTask(const QString &taskId, const QString &taskType, const QVariantMap &parameters)
{
    // 找到合适的插件
    Framework::Plugin *plugin = findBestPluginForTask(taskType);
    if (!plugin) {
        qWarning() << "No plugin supports task type:" << taskType;
        return false;
    }
    
    // 记录任务分配
    m_taskToPlugin[taskId] = plugin->name();
    
    // 执行任务
    plugin->executeTask(taskId, taskType, parameters);
    
    return true;
}

bool PluginManager::cancelTask(const QString &taskId)
{
    if (!m_taskToPlugin.contains(taskId)) {
        qWarning() << "Task not found:" << taskId;
        return false;
    }
    
    QString pluginName = m_taskToPlugin[taskId];
    Framework::Plugin *plugin = m_pluginLoader->getPlugin(pluginName);
    
    if (!plugin) {
        qWarning() << "Plugin not found:" << pluginName;
        m_taskToPlugin.remove(taskId);
        return false;
    }
    
    plugin->cancelTask(taskId);
    m_taskToPlugin.remove(taskId);
    
    return true;
}

Framework::Plugin* PluginManager::findBestPluginForTask(const QString &taskType) const
{
    // 简单实现，找到第一个支持该任务类型的插件
    for (auto plugin : m_pluginLoader->getAllPlugins()) {
        if (plugin->supportsTaskType(taskType)) {
            return plugin;
        }
    }
    
    return nullptr;
} 