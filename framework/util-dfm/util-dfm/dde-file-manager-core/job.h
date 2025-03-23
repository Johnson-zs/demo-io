#ifndef DFM_JOB_H
#define DFM_JOB_H

#include <QObject>

namespace DFM {

// 任务标志，与commands.h中的JobFlag同步
typedef int JobFlags;

/**
 * @class Job
 * @brief 任务基类
 * 
 * Job类是所有任务的基类，提供基本的任务管理功能。
 */
class Job : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString errorText READ errorText)
    Q_PROPERTY(int error READ error)
    Q_PROPERTY(bool running READ isRunning)

public:
    virtual ~Job();

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

public Q_SLOTS:
    /**
     * @brief 启动任务
     * 
     * 开始执行任务
     */
    void start();
    
    /**
     * @brief 挂起任务
     * 
     * 暂停任务执行
     * @return 如果成功返回true
     */
    bool suspend();
    
    /**
     * @brief 恢复任务
     * 
     * 恢复挂起的任务
     * @return 如果成功返回true
     */
    bool resume();
    
    /**
     * @brief 取消任务
     * 
     * 终止任务执行
     * @return 如果成功返回true
     */
    bool kill(bool quietly = false);

Q_SIGNALS:
    /**
     * @brief 当任务完成时发出的信号
     * @param job 任务指针
     */
    void result(Job *job);
    
    /**
     * @brief 当任务被挂起时发出的信号
     * @param job 任务指针
     */
    void suspended(Job *job);
    
    /**
     * @brief 当任务被恢复时发出的信号
     * @param job 任务指针
     */
    void resumed(Job *job);
    
    /**
     * @brief 当任务状态发生变化时发出的信号
     * @param job 任务指针
     */
    void statusChanged(Job *job);
    
    /**
     * @brief 当出现警告时发出的信号
     * @param job 任务指针
     * @param message 警告消息
     */
    void warning(Job *job, const QString &message);
    
    /**
     * @brief 当出现信息时发出的信号
     * @param job 任务指针
     * @param message 信息消息
     */
    void infoMessage(Job *job, const QString &message);
    
    /**
     * @brief 当完成百分比发生变化时发出的信号
     * @param job 任务指针
     * @param percent 完成百分比
     */
    void percent(Job *job, unsigned long percent);
    
    /**
     * @brief 当速度发生变化时发出的信号
     * @param job 任务指针
     * @param speed 速度(字节/秒)
     */
    void speed(Job *job, unsigned long speed);

protected:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Job(QObject *parent = nullptr);
    
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