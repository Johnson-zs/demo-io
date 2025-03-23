#ifndef DFM_DU_WORKER_H
#define DFM_DU_WORKER_H

#include <QObject>
#include <workerbase.h>

namespace DFM {

/**
 * @class DUWorker
 * @brief 磁盘使用统计Worker
 * 
 * DUWorker负责计算文件或目录的磁盘使用情况。
 */
class DUWorker : public WorkerBase {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param protocol 协议
     * @param poolSocket 池套接字
     * @param appSocket 应用套接字
     */
    DUWorker(const QByteArray &protocol, const QByteArray &poolSocket, const QByteArray &appSocket);
    
    /**
     * @brief 析构函数
     */
    ~DUWorker() override;

    /**
     * @brief 处理特殊命令
     * @param data 命令数据
     * @return 操作结果
     */
    WorkerResult special(const QByteArray &data) override;
    
    /**
     * @brief 统计磁盘使用情况
     * @param url 文件或目录URL
     * @return 操作结果
     */
    WorkerResult stat(const QUrl &url) override;

private:
    /**
     * @brief 递归计算目录大小
     * @param path 目录路径
     * @return 目录大小(字节)
     */
    qint64 calculateDirSize(const QString &path);
};

} // namespace DFM

#endif // DFM_DU_WORKER_H 