#ifndef DFM_CONNECTION_H
#define DFM_CONNECTION_H

#include <QObject>
#include <QByteArray>
#include <memory>

namespace DFM {

class ConnectionBackend;
class ConnectionPrivate;
struct Task;

/**
 * @class Connection
 * @brief 提供主程序和Worker之间的通信机制
 * 
 * Connection类处理两个进程之间的数据传输，负责发送命令和接收响应。
 * 这个类使用域套接字实现进程间通信。
 */
class Connection : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 连接类型枚举
     */
    enum class Type {
        Application,  ///< 在应用程序端的连接
        Worker        ///< 在Worker端的连接
    };
    
    /**
     * @brief 读取模式枚举
     */
    enum class ReadMode {
        EventDriven,  ///< 事件驱动模式，通过信号通知数据可读
        WaitForData   ///< 等待数据模式，通过阻塞等待数据
    };

    /**
     * @brief 构造函数
     * @param type 连接类型
     * @param parent 父对象
     */
    explicit Connection(Type type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~Connection();

    /**
     * @brief 挂起连接
     * 
     * 挂起连接将暂停数据的接收，但不会影响发送
     */
    void suspend();
    
    /**
     * @brief 恢复连接
     * 
     * 恢复之前挂起的连接
     */
    void resume();
    
    /**
     * @brief 关闭连接
     * 
     * 关闭连接并释放资源
     */
    void close();

    /**
     * @brief 检查连接是否已建立
     * @return 如果连接已建立，返回true
     */
    bool isConnected() const;
    
    /**
     * @brief 检查连接是否已初始化
     * @return 如果连接已初始化，返回true
     */
    bool inited() const;
    
    /**
     * @brief 检查连接是否已挂起
     * @return 如果连接已挂起，返回true
     */
    bool suspended() const;

    /**
     * @brief 连接到远程端点
     * @param address 远程端点地址
     */
    void connectToRemote(const QUrl &address);

    /**
     * @brief 监听远程连接
     * @return 如果监听成功，返回true
     */
    void listenForRemote();

    /**
     * @brief 获取下一个等待的连接
     * @return 新的连接实例，如果没有连接则返回nullptr
     */
    Connection *nextPendingConnection();

    /**
     * @brief 设置挂起状态
     * @param enable 是否启用挂起
     */
    void setSuspended(bool enable);

    /**
     * @brief 发送命令到远程端点
     * @param cmd 命令ID
     * @param data 命令数据
     * @return 如果发送成功，返回true
     */
    bool send(int cmd, const QByteArray &data);
    
    /**
     * @brief 立即发送命令到远程端点
     * @param cmd 命令ID
     * @param data 命令数据
     * @return 如果发送成功，返回true
     * 
     * 与send不同，sendnow会立即发送数据，不会缓存
     */
    bool sendnow(int cmd, const QByteArray &data);

    /**
     * @brief 检查是否有可用任务
     * @return 如果有可用任务，返回true
     */
    bool hasTaskAvailable() const;
    
    /**
     * @brief 等待接收任务
     * @param ms 超时时间(毫秒)，-1表示无限等待
     * @return 如果成功接收任务，返回true
     */
    bool waitForIncomingTask(int ms);

    /**
     * @brief 读取数据
     * @param _cmd 输出参数，接收到的命令ID
     * @param data 输出参数，接收到的数据
     * @return 读取的数据大小，-1表示出错
     */
    int read(int *_cmd, QByteArray &data);
    
    /**
     * @brief 设置读取模式
     * @param readMode 读取模式
     */
    void setReadMode(ReadMode readMode);

    /**
     * @brief 处理发送任务队列
     */
    void processOutgoingTasks();

    /**
     * @brief 读取命令
     * @return 读取的任务
     */
    Task readCommand();

Q_SIGNALS:
    /**
     * @brief 当有数据可读时发出的信号
     */
    void readyRead();
    
    /**
     * @brief 当连接断开时发出的信号
     */
    void disconnected();
    
    /**
     * @brief 当有新连接时发出的信号
     */
    void newConnection();

private Q_SLOTS:
    /**
     * @brief 处理接收到的任务
     * @param task 任务数据
     */
    void slotGotTask(const Task &task);
    
    /**
     * @brief 处理后端断开连接事件
     */
    void slotBackendDisconnected();
    
    /**
     * @brief 处理新连接事件
     */
    void slotNewConnection();

private:
    friend class Worker;
    friend class ConnectionServer;
    
    std::unique_ptr<ConnectionPrivate> d;
    Type m_type;
};

} // namespace DFM

#endif // DFM_CONNECTION_H 