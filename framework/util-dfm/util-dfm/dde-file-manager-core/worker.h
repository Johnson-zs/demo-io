#ifndef DFM_WORKER_H
#define DFM_WORKER_H

#include <QObject>
#include <QElapsedTimer>

class QUrl;

namespace DFM {

class WorkerThread;
class SimpleJob;
class Connection;
class ConnectionServer;

/**
 * @class Worker
 * @brief 表示Worker实例的类
 * 
 * Worker类表示一个Worker实例，它可以是一个进程或线程。
 * 它封装了Worker的生命周期管理和通信。
 */
class Worker : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param protocol 协议
     * @param parent 父对象
     */
    explicit Worker(const QString &protocol, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~Worker();

    /**
     * @brief 获取协议
     * @return 协议
     */
    QString protocol() const;
    
    /**
     * @brief 设置协议
     * @param protocol 协议
     */
    void setProtocol(const QString &protocol);
    
    /**
     * @brief 获取Worker协议
     * @return Worker协议
     */
    QString workerProtocol() const;

    /**
     * @brief 获取主机
     * @return 主机
     */
    QString host() const;
    
    /**
     * @brief 获取端口
     * @return 端口
     */
    quint16 port() const;
    
    /**
     * @brief 获取用户名
     * @return 用户名
     */
    QString user() const;
    
    /**
     * @brief 获取密码
     * @return 密码
     */
    QString passwd() const;

    /**
     * @brief 设置Worker为空闲状态
     */
    void setIdle();
    
    /**
     * @brief 增加引用计数
     */
    void ref();
    
    /**
     * @brief 减少引用计数
     */
    void deref();
    
    /**
     * @brief 即将删除时的清理工作
     */
    virtual void aboutToDelete();

    /**
     * @brief 设置Worker线程
     * @param thread Worker线程
     */
    void setWorkerThread(WorkerThread *thread);
    
    /**
     * @brief 获取空闲时间
     * @return 空闲时间(秒)
     */
    int idleTime() const;

    /**
     * @brief 设置进程ID
     * @param pid 进程ID
     */
    void setPID(qint64 pid);
    
    /**
     * @brief 获取Worker进程ID
     * @return 进程ID
     */
    qint64 worker_pid() const;

    /**
     * @brief 设置任务
     * @param job 任务
     */
    void setJob(SimpleJob *job);
    
    /**
     * @brief 获取任务
     * @return 任务
     */
    SimpleJob *job() const;

    /**
     * @brief 检查Worker是否活跃
     * @return 如果活跃返回true
     */
    bool isAlive() const;

    /**
     * @brief 挂起Worker
     */
    void suspend();
    
    /**
     * @brief 恢复Worker
     */
    void resume();
    
    /**
     * @brief 检查Worker是否已挂起
     * @return 如果已挂起返回true
     */
    bool suspended();

    /**
     * @brief 发送命令到Worker
     * @param cmd 命令ID
     * @param arr 命令数据
     */
    void send(int cmd, const QByteArray &arr);

    /**
     * @brief 设置主机信息
     * @param host 主机名
     * @param port 端口
     * @param user 用户名
     * @param passwd 密码
     */
    void setHost(const QString &host, quint16 port, const QString &user, const QString &passwd);
    
    /**
     * @brief 重置主机信息
     */
    void resetHost();
    
    /**
     * @brief 设置配置
     * @param config 配置
     */
    void setConfig(const QMap<QString, QString> &config);

    /**
     * @brief 创建Worker
     * @param protocol 协议
     * @param url URL
     * @param error 错误码
     * @param error_text 错误文本
     * @return 创建的Worker
     */
    static Worker* createWorker(const QString &protocol, const QUrl &url, int &error, QString &error_text);

Q_SIGNALS:
    /**
     * @brief 当Worker死亡时发出的信号
     * @param worker Worker指针
     */
    void workerDied(Worker *worker);
    
    /**
     * @brief 当发生错误时发出的信号
     * @param errorCode 错误代码
     * @param errorText 错误文本
     */
    void error(int errorCode, const QString &errorText);

private Q_SLOTS:
    /**
     * @brief 接受连接
     */
    void accept();
    
    /**
     * @brief 处理超时
     */
    void timeout();
    
    /**
     * @brief 处理输入数据
     */
    void gotInput();

private:
    /**
     * @brief 杀死Worker
     */
    void kill();

    QString m_protocol;             ///< 协议
    QString m_workerProtocol;       ///< Worker协议
    QString m_host;                 ///< 主机
    quint16 m_port = 0;             ///< 端口
    QString m_user;                 ///< 用户名
    QString m_passwd;               ///< 密码
    qint64 m_pid = 0;               ///< 进程ID
    Connection *m_connection = nullptr;  ///< 连接
    ConnectionServer *m_workerConnServer = nullptr; ///< 连接服务器
    QElapsedTimer m_contact_started;  ///< 连接开始时间
    QElapsedTimer m_idleSince;        ///< 空闲开始时间
    int m_refCount = 0;               ///< 引用计数
    WorkerThread *m_workerThread = nullptr;  ///< Worker线程
    bool m_dead = false;               ///< 是否已死亡
    SimpleJob *m_job = nullptr;        ///< 当前任务
};

} // namespace DFM

#endif // DFM_WORKER_H 