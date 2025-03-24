#ifndef DFM_SIMPLEJOB_H
#define DFM_SIMPLEJOB_H

#include "job.h"
#include "errorcodes.h"
#include <QUrl>

namespace DFM {

class Worker;
class SimpleJobPrivate;

/**
 * @class SimpleJob
 * @brief 简单任务的基类
 * 
 * SimpleJob是一个基本的任务类，表示单个URL上的简单操作。
 * 它是所有具体任务类型的基类。
 */
class SimpleJob : public Job {
    Q_OBJECT

public:
    virtual ~SimpleJob();

    /**
     * @brief 实现Job::start
     */
    void start() override;
    
    /**
     * @brief 实现Job::cancel
     */
    void cancel() override;
    
    /**
     * @brief 实现Job::suspend
     */
    void suspend() override;
    
    /**
     * @brief 实现Job::resume
     */
    void resume() override;

    /**
     * @brief 获取任务的URL
     * @return 任务URL
     */
    const QUrl &url() const;
    
    /**
     * @brief 设置任务的URL
     * @param url 任务URL
     */
    void setUrl(const QUrl &url);
    
    /**
     * @brief 获取任务关联的Worker
     * @return Worker指针
     */
    Worker *worker() const;
    
    /**
     * @brief 设置任务关联的Worker
     * @param worker Worker指针
     */
    void setWorker(Worker *worker);

    /**
     * @brief 挂起任务
     * 
     * 挂起任务以便稍后恢复
     */
    virtual void putOnHold();
    
    /**
     * @brief 移除挂起的任务
     * 
     * 移除处于挂起状态的任务
     */
    static void removeOnHold();

    /**
     * @brief 检查是否启用重定向处理
     * @return 如果启用返回true
     */
    bool isRedirectionHandlingEnabled() const;
    
    /**
     * @brief 设置是否启用重定向处理
     * @param handle 是否启用
     */
    void setRedirectionHandlingEnabled(bool handle);

public Q_SLOTS:
    /**
     * @brief 处理Worker错误
     * @param error 错误码
     * @param errorText 错误文本
     */
    void slotError(int error, const QString &errorText);

protected Q_SLOTS:
    /**
     * @brief 处理任务完成
     */
    virtual void slotFinished();
    
    /**
     * @brief 处理Worker警告
     * @param warning 警告文本
     */
    virtual void slotWarning(const QString &warning);
    
    /**
     * @brief 处理元数据
     * @param metaData 元数据
     */
    virtual void slotMetaData(const QMap<QString, QString> &metaData);

protected:
    /**
     * @brief 构造函数
     * @param url 任务URL
     * @param parent 父对象
     */
    SimpleJob(const QUrl &url, QObject *parent = nullptr);

private:
    friend class Scheduler;
    std::unique_ptr<SimpleJobPrivate> d;
};

/**
 * @brief 创建自定义的SimpleJob子类
 * 
 * 提供一个自定义的SimpleJob子类，解决抽象类问题
 */
class CustomSimpleJob : public SimpleJob {
public:
    explicit CustomSimpleJob(const QUrl &url, QObject *parent = nullptr);
};

/**
 * @brief 创建文件删除任务
 * @param url 要删除的文件URL
 * @param flags 任务标志
 * @return 创建的任务
 */
SimpleJob *file_delete(const QUrl &url, JobFlags flags = JOB_DEFAULT);

/**
 * @brief 创建目录删除任务
 * @param url 要删除的目录URL
 * @return 创建的任务
 */
SimpleJob *rmdir(const QUrl &url);

/**
 * @brief 创建更改权限任务
 * @param url 目标文件URL
 * @param permissions 权限
 * @return 创建的任务
 */
SimpleJob *chmod(const QUrl &url, int permissions);

/**
 * @brief 创建更改所有者任务
 * @param url 目标文件URL
 * @param owner 新所有者
 * @param group 新组
 * @return 创建的任务
 */
SimpleJob *chown(const QUrl &url, const QString &owner, const QString &group);

/**
 * @brief 创建设置修改时间任务
 * @param url 目标文件URL
 * @param mtime 修改时间
 * @return 创建的任务
 */
SimpleJob *setModificationTime(const QUrl &url, const QDateTime &mtime);

/**
 * @brief 创建重命名任务
 * @param src 源URL
 * @param dest 目标URL
 * @param flags 任务标志
 * @return 创建的任务
 */
SimpleJob *rename(const QUrl &src, const QUrl &dest, JobFlags flags = JOB_DEFAULT);

/**
 * @brief 创建符号链接任务
 * @param target 链接目标
 * @param dest 链接URL
 * @param flags 任务标志
 * @return 创建的任务
 */
SimpleJob *symlink(const QString &target, const QUrl &dest, JobFlags flags = JOB_DEFAULT);

/**
 * @brief 创建特殊命令任务
 * @param url 目标URL
 * @param data 命令数据
 * @param flags 任务标志
 * @return 创建的任务
 */
SimpleJob *special(const QUrl &url, const QByteArray &data, JobFlags flags = JOB_DEFAULT);

/**
 * @brief 创建磁盘使用统计任务
 * @param url 目标URL
 * @param recursive 是否递归
 * @param flags 任务标志
 * @return 创建的任务
 */
SimpleJob *du(const QUrl &url, bool recursive = false, JobFlags flags = JOB_DEFAULT);

} // namespace DFM

#endif // DFM_SIMPLEJOB_H 