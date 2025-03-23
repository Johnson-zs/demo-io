#include "workerfactory.h"
#include "duworker.h"
#include <QDebug>

namespace DFM {

/**
 * @brief WorkerFactory构造函数
 * @param parent 父对象
 */
WorkerFactory::WorkerFactory(QObject *parent)
    : QObject(parent)
{
    qDebug() << "DU WorkerFactory: Created";
}

/**
 * @brief WorkerFactory析构函数
 */
WorkerFactory::~WorkerFactory()
{
    qDebug() << "DU WorkerFactory: Destroyed";
}

/**
 * @brief 创建DUWorker实例
 * @param protocol 协议
 * @param poolSocket 池套接字
 * @param appSocket 应用套接字
 * @return DUWorker实例
 */
QObject *WorkerFactory::createWorker(const QByteArray &protocol, 
                                   const QByteArray &poolSocket,
                                   const QByteArray &appSocket)
{
    qDebug() << "DU WorkerFactory: Creating worker for protocol" << protocol;
    return new DUWorker(protocol, poolSocket, appSocket);
}

// 工厂函数导出 
extern "C" {

/**
 * @brief 工厂函数，用于创建DUWorker实例
 * @param protocol 协议
 * @param poolSocket 池套接字
 * @param appSocket 应用套接字
 * @return DUWorker实例
 */
Q_DECL_EXPORT QObject *create_worker(const QByteArray &protocol,
                                    const QByteArray &poolSocket,
                                    const QByteArray &appSocket)
{
    return WorkerFactory::createWorker(protocol, poolSocket, appSocket);
}

}

} // namespace DFM 