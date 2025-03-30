#pragma once

#include <QObject>
#include <framework/worker_factory.h>

/**
 * @brief 图像处理工厂类
 * 
 * 负责创建ImageProcessor实例和提供Worker元数据
 */
class ImageProcessorFactory : public QObject, public Framework::WorkerFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WORKER_FACTORY_IID FILE "image_processor.json")
    Q_INTERFACES(Framework::WorkerFactory)
    
public:
    ImageProcessorFactory() = default;
    ~ImageProcessorFactory() override = default;
    
    // WorkerFactory接口实现
    QString name() const override;
    QString description() const override;
    QString version() const override;
    QStringList supportedTaskTypes() const override;
    Framework::WorkerBase* createWorker(QObject* parent = nullptr) override;
}; 