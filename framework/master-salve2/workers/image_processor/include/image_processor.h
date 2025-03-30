#pragma once

#include <framework/worker_base.h>
#include <QTimer>
#include <QMap>

/**
 * @brief 任务数据结构
 */
struct TaskData {
    QString taskId;
    QString taskType;
    QVariantMap parameters;
    int progress;
    bool stopped;
    QTimer *timer;
};

/**
 * @brief 图像处理器Worker实现
 * 
 * 负责处理图像相关任务，包括调整大小、裁剪、模糊等操作
 */
class ImageProcessor : public Framework::WorkerBase
{
    Q_OBJECT
    
public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor() override;
    
    // WorkerBase接口实现
    bool initialize() override;
    bool processTask(const Framework::TaskMessage &task) override;
    void cancelTask(const QString &taskId) override;
    void terminate() override;
    
private slots:
    void updateProgress();
    
private:
    void processImage(TaskData *task);
    void cleanupTask(TaskData *task);
    
    QMap<QString, TaskData*> m_runningTasks;
}; 