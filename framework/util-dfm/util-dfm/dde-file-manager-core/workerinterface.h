#ifndef DFM_WORKERINTERFACE_H
#define DFM_WORKERINTERFACE_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QUrl>
#include <QVariantMap>

namespace DFM {

class Connection;
using MetaData = QMap<QString, QString>;

/**
 * @class WorkerInterface
 * @brief Worker接口基类，定义了Worker的基本操作
 * 
 * WorkerInterface类提供了Worker通信的接口，
 * 主要负责Worker的命令分发和事件处理。
 */
class WorkerInterface : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WorkerInterface(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~WorkerInterface();

    /**
     * @brief 分发命令
     * @return 如果分发成功返回true
     */
    bool dispatch();
    
    /**
     * @brief 分发指定命令
     * @param cmd 命令ID
     * @param data 命令数据
     * @return 如果分发成功返回true
     */
    bool dispatch(int cmd, const QByteArray &data);
    
    /**
     * @brief 设置文件偏移量
     * @param o 偏移量
     */
    void setOffset(qint64 o);
    
    /**
     * @brief 获取文件偏移量
     * @return 当前偏移量
     */
    qint64 offset() const;
    
    /**
     * @brief 发送恢复传输答复
     * @param resume 是否可以恢复
     */
    void sendResumeAnswer(bool resume);
    
    /**
     * @brief 发送消息框答复
     * @param result 消息框结果
     */
    void sendMessageBoxAnswer(int result);

Q_SIGNALS:
    /**
     * @brief 当有数据可读时发出的信号
     * @param data 接收到的数据
     */
    void data(const QByteArray &data);
    
    /**
     * @brief 当需要数据时发出的信号
     */
    void dataReq();
    
    /**
     * @brief 当连接打开时发出的信号
     */
    void open();
    
    /**
     * @brief 当任务完成时发出的信号
     */
    void finished();
    
    /**
     * @brief 当接收到文件属性时发出的信号
     * @param entry 文件属性条目
     */
    void statEntry(const QVariantMap &entry);
    
    /**
     * @brief 当接收到文件列表时发出的信号
     * @param list 文件属性列表
     */
    void listEntries(const QList<QVariantMap> &list);
    
    /**
     * @brief 当可以恢复传输时发出的信号
     * @param offset 可以恢复的偏移量
     */
    void canResume(qint64 offset);
    
    /**
     * @brief 当发生错误时发出的信号
     * @param errCode 错误代码
     * @param errorText 错误文本
     */
    void error(int errCode, const QString &errorText);
    
    /**
     * @brief 当Worker状态改变时发出的信号
     * @param pid 进程ID
     * @param protocol 协议
     * @param host 主机名
     * @param connected 是否已连接
     */
    void workerStatus(qint64 pid, const QByteArray &protocol, const QString &host, bool connected);
    
    /**
     * @brief 当连接建立时发出的信号
     */
    void connected();
    
    /**
     * @brief 当写入数据时发出的信号
     * @param size 写入的数据大小
     */
    void written(qint64 size);
    
    /**
     * @brief 当总大小确定时发出的信号
     * @param size 总大小
     */
    void totalSize(qint64 size);
    
    /**
     * @brief 当处理的数据大小改变时发出的信号
     * @param size 已处理的数据大小
     */
    void processedSize(qint64 size);
    
    /**
     * @brief 当位置改变时发出的信号
     * @param pos 新位置
     */
    void position(qint64 pos);
    
    /**
     * @brief 当文件被截断时发出的信号
     * @param length 新长度
     */
    void truncated(qint64 length);
    
    /**
     * @brief 当传输速度改变时发出的信号
     * @param bytes_per_second 每秒字节数
     */
    void speed(unsigned long bytes_per_second);
    
    /**
     * @brief 当重定向时发出的信号
     * @param url 新URL
     */
    void redirection(const QUrl &url);
    
    /**
     * @brief 当MIME类型确定时发出的信号
     * @param type MIME类型
     */
    void mimeType(const QString &type);
    
    /**
     * @brief 当有警告时发出的信号
     * @param msg 警告消息
     */
    void warning(const QString &msg);
    
    /**
     * @brief 当有信息消息时发出的信号
     * @param msg 信息消息
     */
    void infoMessage(const QString &msg);
    
    /**
     * @brief 当有元数据时发出的信号
     * @param m 元数据
     */
    void metaData(const MetaData &m);

protected:
    /**
     * @brief 显示消息框
     * @param type 消息框类型
     * @param text 消息文本
     * @param title 标题
     * @param primaryActionText 主要操作文本
     * @param secondaryActionText 次要操作文本
     */
    virtual void messageBox(int type, const QString &text, const QString &title, 
                          const QString &primaryActionText, const QString &secondaryActionText);
    
    /**
     * @brief 显示消息框(带不再询问)
     * @param type 消息框类型
     * @param text 消息文本
     * @param title 标题
     * @param primaryActionText 主要操作文本
     * @param secondaryActionText 次要操作文本
     * @param dontAskAgainName 不再询问标识
     */
    virtual void messageBox(int type, const QString &text, const QString &title,
                          const QString &primaryActionText, const QString &secondaryActionText,
                          const QString &dontAskAgainName);

protected Q_SLOTS:
    /**
     * @brief 处理主机信息
     * @param info 主机信息
     */
    void slotHostInfo(const QHostInfo &info);
    
    /**
     * @brief 计算传输速度
     */
    void calcSpeed();

protected:
    Connection *m_connection = nullptr;  ///< 连接对象
    
    // 用于速度计算
    qint64 m_offset = 0;
    qint64 m_filesize = 0;
    bool m_worker_calcs_speed = false;
    
    static const unsigned int max_nums = 8;
    qint64 m_times[max_nums];
    qint64 m_sizes[max_nums];
    unsigned int m_nums = 0;
    
    qint64 m_start_time = 0;
    qint64 m_last_time = 0;
    QTimer m_speed_timer;
    
    QString m_messageBoxDetails;  ///< 消息框详情
};

} // namespace DFM

#endif // DFM_WORKERINTERFACE_H 