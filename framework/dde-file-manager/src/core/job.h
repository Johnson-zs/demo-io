#pragma once

#include <QObject>
#include <QUrl>
#include <QString>
#include <QVariant>
#include <QPromise>
#include <QFuture>
#include <memory>

namespace DFM {

// 任务状态枚举
enum class JobState {
    None,
    Starting,
    Running,
    Paused,
    Canceled,
    Finished,
    Error
};

// 任务类型枚举
enum class JobType {
    Generic,
    DiskUsage,
    FileSearch,
    FileTransfer,
    // 其他任务类型...
};

// 任务结果抽象基类
class JobResult {
public:
    virtual ~JobResult() = default;
    virtual bool isSuccess() const = 0;
    virtual QString errorString() const = 0;
};

// 任务基类
class Job : public QObject {
    Q_OBJECT
    Q_PROPERTY(JobState state READ state NOTIFY stateChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    // 构造函数
    explicit Job(QObject *parent = nullptr);
    virtual ~Job() = default;

    // 基本属性
    JobState state() const;
    int progress() const;
    QString errorString() const;
    virtual JobType type() const = 0;

    // 控制方法
    virtual void start() = 0;
    virtual bool suspend();
    virtual bool resume();
    virtual bool cancel();

    // 异步结果（使用Qt6的QFuture）
    template<typename T>
    QFuture<T> future() const;

signals:
    void stateChanged(JobState state);
    void progressChanged(int progress);
    void errorStringChanged(const QString &errorString);
    void finished(bool success);
    void dataReceived(const QVariant &data);

protected:
    void setState(JobState state);
    void setProgress(int progress);
    void setErrorString(const QString &errorString);

private:
    class Private;
    std::unique_ptr<Private> d;
};

// SimpleJob是与单个Worker通信的基本任务
class SimpleJob : public Job {
    Q_OBJECT
public:
    explicit SimpleJob(const QUrl &url, QObject *parent = nullptr);
    ~SimpleJob() override;

    QUrl url() const;
    void setWorker(std::shared_ptr<class Worker> worker);

protected:
    void sendCommand(int cmd, const QByteArray &data = QByteArray());
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace DFM 