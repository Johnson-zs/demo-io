#include "../include/image_processor.h"
#include <QDebug>
#include <QThread>

ImageProcessor::ImageProcessor(QObject *parent)
    : Framework::WorkerBase(parent)
{
}

ImageProcessor::~ImageProcessor()
{
    terminate();
}

bool ImageProcessor::initialize()
{
    qInfo() << "Initializing ImageProcessor plugin";
    return true;
}

bool ImageProcessor::processTask(const Framework::TaskMessage &task)
{
    QString taskId = task.taskId;
    QString taskType = task.taskType;
    QVariantMap parameters = task.parameters;
    
    // 检查任务类型
    if (taskType != "ImageProcessing") {
        error(taskId, 1, "Unsupported task type: " + taskType);
        return false;
    }
    
    // 创建任务数据
    TaskData *taskData = new TaskData();
    taskData->taskId = taskId;
    taskData->taskType = taskType;
    taskData->parameters = parameters;
    taskData->progress = 0;
    taskData->stopped = false;
    taskData->timer = new QTimer(this);
    
    // 连接进度更新定时器
    connect(taskData->timer, &QTimer::timeout, this, &ImageProcessor::updateProgress);
    taskData->timer->start(500); // 每500毫秒更新一次进度
    
    // 添加到任务列表
    m_runningTasks[taskId] = taskData;
    
    qInfo() << "Started image processing task:" << taskId;
    
    // 开始处理图像
    processImage(taskData);
    
    return true;
}

void ImageProcessor::cancelTask(const QString &taskId)
{
    if (!m_runningTasks.contains(taskId)) {
        qWarning() << "Task not found:" << taskId;
        return;
    }
    
    TaskData *task = m_runningTasks[taskId];
    task->stopped = true;
    
    qInfo() << "Canceled image processing task:" << taskId;
    
    // 清理任务
    cleanupTask(task);
    m_runningTasks.remove(taskId);
}

void ImageProcessor::terminate()
{
    qInfo() << "Terminating ImageProcessor plugin";
    
    // 取消所有运行中的任务
    for (auto it = m_runningTasks.begin(); it != m_runningTasks.end(); ++it) {
        TaskData *task = it.value();
        task->stopped = true;
        cleanupTask(task);
    }
    
    m_runningTasks.clear();
}

QStringList ImageProcessor::capabilities() const
{
    return QStringList() << "ImageProcessing";
}

void ImageProcessor::updateProgress()
{
    // 更新所有任务的进度
    for (auto it = m_runningTasks.begin(); it != m_runningTasks.end();) {
        TaskData *task = it.value();
        
        if (task->stopped) {
            ++it;
            continue;
        }
        
        // 增加进度，每次增加10%
        task->progress += 10;
        
        // 发送进度更新
        sendProgress(task->taskId, task->progress);
        
        // 如果完成
        if (task->progress >= 100) {
            // 停止定时器
            task->timer->stop();
            
            // 完成任务
            QVariantMap result;
            result["message"] = QString("Processed image %1 with %2 operation")
                               .arg(task->parameters.value("input_file").toString())
                               .arg(task->parameters.value("operation").toString());
            
            // 发送完成消息
            finished(task->taskId, true, result);
            
            // 清理任务
            cleanupTask(task);
            it = m_runningTasks.erase(it);
        } else {
            ++it;
        }
    }
}

void ImageProcessor::processImage(TaskData *task)
{
    // 模拟图像处理，仅打印一些信息
    QString inputFile = task->parameters.value("input_file").toString();
    QString operation = task->parameters.value("operation").toString();
    
    qInfo() << "Processing image:" << inputFile;
    qInfo() << "Operation:" << operation;
    
    if (task->parameters.contains("width") && task->parameters.contains("height")) {
        int width = task->parameters.value("width").toInt();
        int height = task->parameters.value("height").toInt();
        qInfo() << "Target size:" << width << "x" << height;
    }
    
    // 实际处理逻辑会在updateProgress中逐步执行
}

void ImageProcessor::cleanupTask(TaskData *task)
{
    if (task->timer) {
        task->timer->stop();
        task->timer->deleteLater();
    }
    
    delete task;
} 