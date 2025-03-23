#ifndef DFM_DU_WORKER_FACTORY_H
#define DFM_DU_WORKER_FACTORY_H

#include <QObject>

namespace DFM {

/**
 * @class WorkerFactory
 * @brief DU Worker工厂类
 * 
 * WorkerFactory负责创建DUWorker实例。
 * 此类包含一个入口点函数，用于创建Worker。
 */
class WorkerFactory : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WorkerFactory(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~WorkerFactory() override;

    /**
     * @brief 创建DUWorker实例
     * @param protocol 协议
     * @param poolSocket 池套接字
     * @param appSocket 应用套接字
     * @return DUWorker实例
     */
    static QObject *createWorker(const QByteArray &protocol, 
                               const QByteArray &poolSocket,
                               const QByteArray &appSocket);
};

} // namespace DFM

#endif // DFM_DU_WORKER_FACTORY_H 