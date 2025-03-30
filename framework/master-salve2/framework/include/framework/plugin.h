#pragma once

#include <QString>
#include <QVariant>
#include <QObject>

namespace Framework {

/**
 * @brief 插件接口类，所有Worker插件必须实现此接口
 */
class Plugin : public QObject {
    Q_OBJECT
public:
    explicit Plugin(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Plugin() = default;

    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取插件描述
     * @return 插件描述
     */
    virtual QString description() const = 0;

    /**
     * @brief 获取插件版本
     * @return 插件版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 初始化插件
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 关闭插件
     */
    virtual void shutdown() = 0;

    /**
     * @brief 检查插件是否支持指定任务类型
     * @param taskType 任务类型
     * @return 是否支持
     */
    virtual bool supportsTaskType(const QString &taskType) const = 0;

    /**
     * @brief 执行任务
     * @param taskId 任务ID
     * @param taskType 任务类型
     * @param parameters 任务参数
     */
    virtual void executeTask(const QString &taskId, const QString &taskType, const QVariantMap &parameters) = 0;

    /**
     * @brief 取消任务
     * @param taskId 任务ID
     */
    virtual void cancelTask(const QString &taskId) = 0;

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
};

// 定义插件接口宏
#define PLUGIN_IID "org.example.Plugin"
#define PLUGIN_INTERFACE_VERSION "1.0"

} // namespace Framework

Q_DECLARE_INTERFACE(Framework::Plugin, PLUGIN_IID) 