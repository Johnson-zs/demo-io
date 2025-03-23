#ifndef DFM_WORKER_P_H
#define DFM_WORKER_P_H

#include <QString>
#include <QTime>

namespace DFM {

class Worker;
class Connection;

/**
 * @brief Worker的私有类
 */
class WorkerPrivate
{
public:
    /**
     * @brief Worker状态
     */
    enum State {
        Idle,       ///< 空闲状态
        Launching,  ///< 正在启动
        Running,    ///< 运行中
        Failed      ///< 启动失败
    };

    /**
     * @brief 构造函数
     * @param parent 所属的Worker对象
     */
    explicit WorkerPrivate(Worker *parent);

    /**
     * @brief 析构函数
     */
    ~WorkerPrivate();

    /**
     * @brief 设置连接
     * @param conn 连接对象
     */
    void setConnection(Connection *conn);

    /**
     * @brief 启动Worker
     */
    void startWorker();

    /**
     * @brief 所属的Worker对象
     */
    Worker *q;

    /**
     * @brief 连接对象
     */
    Connection *connection;

    /**
     * @brief 协议名称
     */
    QString protocol;

    /**
     * @brief 当前状态
     */
    State state;

    /**
     * @brief 是否挂起
     */
    bool onHold;

    /**
     * @brief 引用计数
     */
    int refCount;

    /**
     * @brief 是否空闲
     */
    bool idle;

    /**
     * @brief 空闲开始时间
     */
    QTime idleStart;
};

} // namespace DFM

#endif // DFM_WORKER_P_H 