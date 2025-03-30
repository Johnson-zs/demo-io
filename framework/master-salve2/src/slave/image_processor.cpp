#include "image_processor.h"
#include <QDebug>
#include <QThread>

ImageProcessor::ImageProcessor(const TaskMessage &task, QObject *parent)
    : TaskExecutor(task, parent)
{
}

void ImageProcessor::start()
{
    if (task.taskType != "ImageProcessing") {
        emit completed(task.taskId, false, "Invalid task type");
        return;
    }

    // 获取任务参数
    QString inputFile = task.parameters["input_file"].toString();
    QString operation = task.parameters["operation"].toString();
    int width = task.parameters["width"].toInt();
    int height = task.parameters["height"].toInt();

    qDebug() << "Processing image:" << inputFile;
    qDebug() << "Operation:" << operation;
    qDebug() << "Target size:" << width << "x" << height;

    // 模拟图像处理过程
    for (int i = 0; i <= 100 && !stopped; i += 10) {
        QThread::msleep(500);   // 模拟耗时操作
        emit progressUpdated(task.taskId, i);
    }

    if (!stopped) {
        QString result = QString("Processed image %1 with operation %2")
                                 .arg(inputFile)
                                 .arg(operation);
        emit completed(task.taskId, true, result);
    }
}

void ImageProcessor::stop()
{
    stopped = true;
}
