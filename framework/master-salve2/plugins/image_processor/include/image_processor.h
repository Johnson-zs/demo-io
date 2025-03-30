#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <framework/worker_base.h>
#include <QTimer>
#include <QMap>

// 插件接口
#define WORKER_IID "org.example.Worker"

// 任务数据结构
struct TaskData {
    QString taskId;
    QString taskType;
    QVariantMap parameters;
    int progress;
    bool stopped;
    QTimer *timer;
};

/**
 * @brief 图像处理插件
 */
class ImageProcessor : public Framework::WorkerBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WORKER_IID FILE "image_processor.json")
    Q_INTERFACES(Framework::WorkerBase)
    
public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor() override;
    
    // WorkerBase接口
    bool initialize() override;
    bool processTask(const Framework::TaskMessage &task) override;
    void cancelTask(const QString &taskId) override;
    void terminate() override;
    QStringList capabilities() const override;
    
private slots:
    void updateProgress();
    
private:
    void processImage(TaskData *task);
    void cleanupTask(TaskData *task);
    
    QMap<QString, TaskData*> m_runningTasks;
};

#endif // IMAGE_PROCESSOR_H 