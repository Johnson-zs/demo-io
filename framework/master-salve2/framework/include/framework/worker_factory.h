#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace Framework {

class WorkerBase;

/**
 * @brief Worker工厂接口，用于创建Worker实例
 * 
 * 此接口用于分离Worker的元数据和实例创建逻辑，
 * 每个Worker插件需要实现此接口作为Qt插件入口点。
 */
class WorkerFactory {
public:
    virtual ~WorkerFactory() = default;

    /**
     * @brief 获取Worker名称
     * @return Worker名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取Worker描述
     * @return Worker描述
     */
    virtual QString description() const = 0;

    /**
     * @brief 获取Worker版本
     * @return Worker版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 获取Worker支持的任务类型
     * @return 支持的任务类型列表
     */
    virtual QStringList supportedTaskTypes() const = 0;

    /**
     * @brief 创建Worker实例
     * @param parent 父对象
     * @return Worker实例
     */
    virtual WorkerBase* createWorker(QObject* parent = nullptr) = 0;
};

} // namespace Framework

// 定义插件接口ID
#define WORKER_FACTORY_IID "org.framework.WorkerFactory/1.0"
Q_DECLARE_INTERFACE(Framework::WorkerFactory, WORKER_FACTORY_IID) 