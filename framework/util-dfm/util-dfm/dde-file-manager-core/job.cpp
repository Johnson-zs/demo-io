#include "job.h"
#include "job_p.h"
#include "commands.h"
#include <QDebug>

namespace DFM {

Job::Job(QObject *parent)
    : QObject(parent)
    , d(new JobPrivate(this))
{
    // 初始化默认值
    d->operationType = JobPrivate::Other;
}

Job::~Job()
{
    // 取消任务
    if (d->running) {
        kill(true); // quietly
    }
    
    delete d;
}

int Job::error() const
{
    return d->error;
}

QString Job::errorText() const
{
    return d->errorText;
}

bool Job::isRunning() const
{
    return d->running;
}

JobFlags Job::flags() const
{
    return d->jobFlags;
}

void Job::setFlags(JobFlags jobFlags)
{
    d->jobFlags = jobFlags;
}

void Job::start()
{
    // 设置运行状态
    d->running = true;
    
    // 发送状态变化信号
    Q_EMIT statusChanged(this);
}

bool Job::suspend()
{
    if (!d->running || d->suspended) {
        return false;
    }
    
    // 执行子类的挂起逻辑
    if (!doSuspend()) {
        return false;
    }
    
    // 设置挂起状态
    d->suspended = true;
    
    // 发送挂起信号
    Q_EMIT suspended(this);
    Q_EMIT statusChanged(this);
    
    return true;
}

bool Job::resume()
{
    if (!d->running || !d->suspended) {
        return false;
    }
    
    // 执行子类的恢复逻辑
    if (!doResume()) {
        return false;
    }
    
    // 清除挂起状态
    d->suspended = false;
    
    // 发送恢复信号
    Q_EMIT resumed(this);
    Q_EMIT statusChanged(this);
    
    return true;
}

bool Job::kill(bool quietly)
{
    if (!d->running) {
        return false;
    }
    
    // 执行子类的终止逻辑
    if (!doKill()) {
        return false;
    }
    
    // 清除运行状态
    d->running = false;
    d->suspended = false;
    
    if (!quietly) {
        // 发送状态变化信号
        Q_EMIT statusChanged(this);
    }
    
    return true;
}

void Job::setError(int errorCode)
{
    d->error = errorCode;
}

void Job::setErrorText(const QString &errorText)
{
    d->errorText = errorText;
}

void Job::emitResult()
{
    // 标记任务为已完成
    d->running = false;
    d->suspended = false;
    
    // 发送结果信号
    Q_EMIT result(this);
    Q_EMIT statusChanged(this);
}

void Job::emitPercent(unsigned long percent)
{
    // 限制百分比范围
    percent = qMin(percent, 100UL);
    
    // 发送百分比信号
    Q_EMIT this->percent(this, percent);
}

void Job::emitSpeed(unsigned long bytesPerSecond)
{
    // 发送速度信号
    Q_EMIT speed(this, bytesPerSecond);
}

bool Job::doSuspend()
{
    // 默认实现，子类可以重写
    return true;
}

bool Job::doResume()
{
    // 默认实现，子类可以重写
    return true;
}

bool Job::doKill()
{
    // 默认实现，子类可以重写
    return true;
}

QString buildErrorString(int errorCode, const QString &errorText)
{
    QString result;
    
    // 根据错误代码构造基本错误信息
    switch (errorCode) {
        case ERR_CANNOT_CONNECT:
            result = "无法连接到目标服务器";
            break;
        case ERR_CANNOT_AUTHENTICATE:
            result = "身份验证失败";
            break;
        case ERR_WORKER_DIED:
            result = "Worker进程意外终止";
            break;
        case ERR_CANNOT_ENTER_DIRECTORY:
            result = "无法进入目录";
            break;
        case ERR_ACCESS_DENIED:
            result = "访问被拒绝";
            break;
        case ERR_UNKNOWN:
            result = "发生未知错误";
            break;
        case ERR_WORKER_TIMEOUT:
            result = "Worker操作超时";
            break;
        case ERR_UNSUPPORTED_ACTION:
            result = "不支持的操作";
            break;
        case ERR_DISK_FULL:
            result = "磁盘已满";
            break;
        case ERR_FILE_ALREADY_EXIST:
            result = "文件已存在";
            break;
        default:
            result = "错误 #" + QString::number(errorCode);
            break;
    }
    
    // 添加详细错误信息
    if (!errorText.isEmpty()) {
        result += ": " + errorText;
    }
    
    return result;
}

} // namespace DFM 