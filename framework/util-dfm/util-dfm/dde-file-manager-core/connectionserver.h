#ifndef DFM_CONNECTIONSERVER_H
#define DFM_CONNECTIONSERVER_H

#include <QObject>
#include <QUrl>

namespace DFM {

class Connection;
class ConnectionBackend;

/**
 * @class ConnectionServer
 * @brief 连接服务器类，用于接受传入的连接
 * 
 * ConnectionServer类负责监听客户端连接请求，并创建连接对象。
 */
class ConnectionServer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConnectionServer(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~ConnectionServer();

    /**
     * @brief 监听远程连接
     */
    void listenForRemote();
    
    /**
     * @brief 获取服务器地址
     * @return 服务器地址
     */
    QUrl address() const;
    
    /**
     * @brief 检查服务器是否正在监听
     * @return 如果正在监听返回true
     */
    bool isListening() const;
    
    /**
     * @brief 设置下一个等待的连接
     * @param conn 要设置的连接对象
     */
    void setNextPendingConnection(Connection *conn);

Q_SIGNALS:
    /**
     * @brief 当有新连接时发出的信号
     */
    void newConnection();

private:
    ConnectionBackend *backend = nullptr;  ///< 连接后端
};

} // namespace DFM

#endif // DFM_CONNECTIONSERVER_H 