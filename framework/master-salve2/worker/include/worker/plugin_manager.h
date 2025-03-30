#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "framework/plugin.h"
#include "framework/plugin_loader.h"

/**
 * @brief 插件管理器类，负责管理Worker中的插件
 */
class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    /**
     * @brief 加载插件
     * @param pluginDir 插件目录
     * @return 是否成功加载至少一个插件
     */
    bool loadPlugins(const QString &pluginDir);

    /**
     * @brief 卸载所有插件
     */
    void unloadPlugins();

    /**
     * @brief 获取所有已加载插件名称
     * @return 插件名称列表
     */
    QStringList getPluginNames() const;

    /**
     * @brief 获取所有支持的任务类型
     * @return 任务类型列表
     */
    QStringList getSupportedTaskTypes() const;

    /**
     * @brief 检查是否有插件支持指定任务类型
     * @param taskType 任务类型
     * @return 是否支持
     */
    bool supportsTaskType(const QString &taskType) const;

    /**
     * @brief 执行任务
     * @param taskId 任务ID
     * @param taskType 任务类型
     * @param parameters 任务参数
     * @return 是否成功分配给插件
     */
    bool executeTask(const QString &taskId, const QString &taskType, const QVariantMap &parameters);

    /**
     * @brief 取消任务
     * @param taskId 任务ID
     * @return 是否成功取消
     */
    bool cancelTask(const QString &taskId);

signals:
    /**
     * @brief 任务进度更新信号
     * @param taskId 任务ID
     * @param progress 进度（0-100）
     */
    void taskProgress(const QString &taskId, int progress);

    /**
     * @brief 任务完成信号
     * @param taskId 任务ID
     * @param success 是否成功
     * @param result 任务结果
     */
    void taskCompleted(const QString &taskId, bool success, const QVariant &result);

private:
    Framework::PluginLoader *m_pluginLoader;
    QMap<QString, QString> m_taskToPlugin; // 任务ID -> 插件名称

    /**
     * @brief 查找支持指定任务类型的最佳插件
     * @param taskType 任务类型
     * @return 插件指针，未找到时返回nullptr
     */
    Framework::Plugin* findBestPluginForTask(const QString &taskType) const;
}; 