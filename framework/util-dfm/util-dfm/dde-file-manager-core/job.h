#ifndef DFM_JOB_H
#define DFM_JOB_H

#include <QObject>

namespace DFM {

// 任务标志，与commands.h中的JobFlag同步
typedef int JobFlags;

/**
 * @class Job
 * @brief 作业基类
 * 
 * Job是所有作业类型的基类，提供了基本的作业操作接口。
 */
class Job : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString errorText READ errorText)
    Q_PROPERTY(int error READ error)
    Q_PROPERTY(bool running READ isRunning)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Job(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~Job();
    
    /**
     * @brief 启动作业
     */
    virtual void start() = 0;
    
    /**
     * @brief 取消作业
     */
    virtual void cancel() = 0;
    
    /**
     * @brief 挂起作业
     */
    virtual void suspend() = 0;
    
    /**
     * @brief 恢复作业
     */
    virtual void resume() = 0;

    /**
     * @brief 获取错误代码
     * @return 错误代码
     */
    int error() const;
    
    /**
     * @brief 获取错误文本
     * @return 错误文本
     */
    QString errorText() const;
    
    /**
     * @brief 检查任务是否正在运行
     * @return 如果正在运行返回true
     */
    bool isRunning() const;
    
    /**
     * @brief 获取任务标志
     * @return 任务标志
     */
    JobFlags flags() const;
    
    /**
     * @brief 设置任务标志
     * @param flags 任务标志
     */
    void setFlags(JobFlags flags);

Q_SIGNALS:
    /**
     * @brief 当作业完成时发出的信号
     */
    void finished();
    
    /**
     * @brief 当作业被取消时发出的信号
     */
    void canceled();
    
    /**
     * @brief 当作业发生错误时发出的信号
     * @param errorCode 错误码
     * @param errorText 错误文本
     */
    void error(int errorCode, const QString &errorText);
    
    /**
     * @brief 当作业进度更新时发出的信号
     * @param percent 完成百分比
     */
    void percent(int percent);
    
    /**
     * @brief 当作业处理的数据大小更新时发出的信号
     * @param processedSize 已处理大小
     * @param totalSize 总大小
     */
    void processedSize(qint64 processedSize, qint64 totalSize);
    
    /**
     * @brief 当作业处理速度更新时发出的信号
     * @param speed 处理速度（字节/秒）
     */
    void speed(qint64 speed);

protected:
    /**
     * @brief 设置错误代码
     * @param error 错误代码
     */
    void setError(int error);
    
    /**
     * @brief 设置错误文本
     * @param errorText 错误文本
     */
    void setErrorText(const QString &errorText);
    
    /**
     * @brief 发送结果信号
     * 
     * 发送结果信号并标记任务为已完成
     */
    void emitResult();
    
    /**
     * @brief 发送百分比信号
     * @param percent 完成百分比
     */
    void emitPercent(unsigned long percent);
    
    /**
     * @brief 发送速度信号
     * @param speed 速度(字节/秒)
     */
    void emitSpeed(unsigned long speed);

protected:
    /**
     * @brief 实现任务挂起逻辑
     * @return 如果成功返回true
     */
    virtual bool doSuspend();
    
    /**
     * @brief 实现任务恢复逻辑
     * @return 如果成功返回true
     */
    virtual bool doResume();
    
    /**
     * @brief 实现任务终止逻辑
     * @return 如果成功返回true
     */
    virtual bool doKill();

private:
    class JobPrivate *const d;
};

/**
 * @brief 创建错误字符串
 * @param errorCode 错误代码
 * @param errorText 错误文本
 * @return 格式化的错误字符串
 */
QString buildErrorString(int errorCode, const QString &errorText);

} // namespace DFM

#endif // DFM_JOB_H 