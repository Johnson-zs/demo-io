#include "duworker.h"
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <commands.h>

namespace DFM {

/**
 * @brief DUWorker构造函数
 * @param protocol 协议
 * @param poolSocket 池套接字
 * @param appSocket 应用套接字
 */
DUWorker::DUWorker(const QByteArray &protocol, const QByteArray &poolSocket, const QByteArray &appSocket)
    : WorkerBase(protocol, poolSocket, appSocket)
{
    qDebug() << "DUWorker: Created with protocol" << protocol;
}

/**
 * @brief DUWorker析构函数
 */
DUWorker::~DUWorker()
{
    qDebug() << "DUWorker: Destroyed";
}

/**
 * @brief 处理特殊命令
 * @param data 命令数据
 * @return 操作结果
 */
WorkerResult DUWorker::special(const QByteArray &data)
{
    QDataStream stream(data);
    int command;
    stream >> command;
    
    if (command == CMD_DU || command == CMD_DU_RECURSIVE) {
        QUrl url;
        stream >> url;
        
        qDebug() << "DUWorker: Processing DU command for" << url;
        
        if (!url.isValid() || url.scheme() != QLatin1String("file")) {
            return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, "Only local files supported");
        }
        
        QFileInfo fileInfo(url.toLocalFile());
        if (!fileInfo.exists()) {
            return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, "File does not exist");
        }
        
        // 计算文件或目录大小
        qint64 size = 0;
        if (fileInfo.isDir()) {
            if (command == CMD_DU_RECURSIVE) {
                // 递归计算目录大小
                size = calculateDirSize(fileInfo.absoluteFilePath());
            } else {
                // 只计算目录本身的大小
                size = fileInfo.size();
            }
        } else {
            // 文件大小
            size = fileInfo.size();
        }
        
        // 创建结果信息
        QVariantMap entry;
        entry["size"] = size;
        entry["path"] = fileInfo.absoluteFilePath();
        entry["isDir"] = fileInfo.isDir();
        entry["modified"] = fileInfo.lastModified().toMSecsSinceEpoch();
        
        // 发送结果
        statEntry(entry);
        
        return WorkerResult::pass();
    }
    
    return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, "Unsupported special command");
}

/**
 * @brief 统计磁盘使用情况
 * @param url 文件或目录URL
 * @return 操作结果
 */
WorkerResult DUWorker::stat(const QUrl &url)
{
    qDebug() << "DUWorker: Processing stat command for" << url;
    
    if (!url.isValid() || url.scheme() != QLatin1String("file")) {
        return WorkerResult::fail(ERR_UNSUPPORTED_ACTION, "Only local files supported");
    }
    
    QFileInfo fileInfo(url.toLocalFile());
    if (!fileInfo.exists()) {
        return WorkerResult::fail(ERR_CANNOT_ENTER_DIRECTORY, "File does not exist");
    }
    
    // 创建结果信息
    QVariantMap entry;
    entry["size"] = fileInfo.size();
    entry["path"] = fileInfo.absoluteFilePath();
    entry["isDir"] = fileInfo.isDir();
    entry["modified"] = fileInfo.lastModified().toMSecsSinceEpoch();
    
    // 发送结果
    statEntry(entry);
    
    return WorkerResult::pass();
}

/**
 * @brief 递归计算目录大小
 * @param path 目录路径
 * @return 目录大小(字节)
 */
qint64 DUWorker::calculateDirSize(const QString &path)
{
    QDir dir(path);
    qint64 size = 0;
    
    // 获取目录中的所有文件和子目录
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    // 设置总项目数(用于进度报告)
    totalSize(entries.size());
    
    int processed = 0;
    for (const QFileInfo &fileInfo : entries) {
        if (fileInfo.isDir()) {
            // 递归计算子目录大小
            size += calculateDirSize(fileInfo.absoluteFilePath());
        } else {
            // 添加文件大小
            size += fileInfo.size();
        }
        
        // 报告进度
        processedSize(++processed);
        
        // 检查是否被取消
        if (wasKilled()) {
            break;
        }
    }
    
    return size;
}

// Worker入口点
extern "C" {
int kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    // 确保我们有足够的参数
    if (argc < 3) {
        qWarning() << "Usage:" << argv[0] << "<protocol> <socket>";
        return -1;
    }
    
    QByteArray protocol = argv[1];
    QByteArray socket = argv[2];
    
    // 创建worker实例
    DUWorker worker(protocol, QByteArray(), socket);
    
    // 连接到应用程序
    worker.connectWorker(QString::fromLocal8Bit(socket));
    
    // 运行事件循环
    worker.dispatchLoop();
    
    return 0;
}
}

} // namespace DFM 