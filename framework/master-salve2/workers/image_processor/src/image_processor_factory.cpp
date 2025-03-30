#include "../include/image_processor_factory.h"
#include "../include/image_processor.h"

QString ImageProcessorFactory::name() const
{
    return "ImageProcessor";
}

QString ImageProcessorFactory::description() const
{
    return "处理图像任务，包括调整大小、裁剪、模糊等操作";
}

QString ImageProcessorFactory::version() const
{
    return "1.0.0";
}

QStringList ImageProcessorFactory::supportedTaskTypes() const
{
    return QStringList() << "ImageProcessing";
}

Framework::WorkerBase* ImageProcessorFactory::createWorker(QObject* parent)
{
    return new ImageProcessor(parent);
} 