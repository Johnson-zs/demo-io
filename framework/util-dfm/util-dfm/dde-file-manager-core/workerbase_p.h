#ifndef DFM_WORKERBASE_P_H
#define DFM_WORKERBASE_P_H

#include "workerbase.h"

namespace DFM {

class WorkerBridge;  // 前向声明

/**
 * @class WorkerBasePrivate
 * @brief WorkerBase的私有实现
 */
class WorkerBasePrivate {
public:
    /**
     * @brief 构造函数
     * @param protocol 协议
     * @param poolSocket 池套接字
     * @param appSocket 应用套接字
     * @param q 公共类指针
     */
    WorkerBasePrivate(const QByteArray &protocol, const QByteArray &poolSocket, 
                     const QByteArray &appSocket, WorkerBase *q);
    
    /**
     * @brief 获取协议名
     * @return 协议名
     */
    QByteArray protocolName() const;

    QByteArray m_protocol;    ///< 协议名
    WorkerBridge bridge;      ///< 通信桥
    WorkerBase *q;            ///< 指向公共类的指针
};

/**
 * @class WorkerResultPrivate
 * @brief WorkerResult的私有实现
 */
class WorkerResultPrivate {
public:
    bool success;             ///< 是否成功
    int error;                ///< 错误代码
    QString errorString;      ///< 错误信息
};

} // namespace DFM

#endif // DFM_WORKERBASE_P_H 