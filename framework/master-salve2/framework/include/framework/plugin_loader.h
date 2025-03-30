#pragma once

#include "plugin.h"
#include <QString>
#include <QDir>
#include <QPluginLoader>
#include <QMap>
#include <QObject>

namespace Framework {

/**
 * @brief 插件加载器类，负责加载和管理插件
 */
class PluginLoader : public QObject {
    Q_OBJECT
public:
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader();

    /**
     * @brief 加载指定目录中的所有插件
     * @param directory 插件目录
     * @return 成功加载的插件数量
     */
    int loadPlugins(const QDir &directory);

    /**
     * @brief 加载单个插件
     * @param filePath 插件文件路径
     * @return 加载是否成功
     */
    bool loadPlugin(const QString &filePath);

    /**
     * @brief 卸载所有插件
     */
    void unloadAllPlugins();

    /**
     * @brief 获取指定名称的插件
     * @param name 插件名称
     * @return 插件指针，不存在时返回nullptr
     */
    Plugin* getPlugin(const QString &name) const;

    /**
     * @brief 获取所有已加载的插件
     * @return 插件列表
     */
    QList<Plugin*> getAllPlugins() const;

    /**
     * @brief 获取支持指定任务类型的插件
     * @param taskType 任务类型
     * @return 支持该任务类型的插件列表
     */
    QList<Plugin*> getPluginsForTaskType(const QString &taskType) const;

signals:
    /**
     * @brief 插件加载成功信号
     * @param name 插件名称
     */
    void pluginLoaded(const QString &name);

    /**
     * @brief 插件加载失败信号
     * @param path 插件路径
     * @param errorMessage 错误信息
     */
    void pluginLoadFailed(const QString &path, const QString &errorMessage);

private:
    QMap<QString, QPluginLoader*> m_pluginLoaders;
    QMap<QString, Plugin*> m_plugins;
};

} // namespace Framework 