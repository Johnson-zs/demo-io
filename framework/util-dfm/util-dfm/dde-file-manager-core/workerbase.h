#ifndef DFM_WORKERBASE_H
#define DFM_WORKERBASE_H

#include <QObject>
#include <QIODevice>
#include <QDateTime>
#include <QUrl>
#include <QMap>
#include <QString>
#include <QVariantMap>
#include <memory>

namespace DFM {

class WorkerBasePrivate;
using MetaData = QMap<QString, QString>;

/**
 * @class WorkerResult
 * @brief Worker操作结果类
 */
class WorkerResult {
public:
    ~WorkerResult();
    WorkerResult(const WorkerResult &rhs);
    WorkerResult &operator=(const WorkerResult &rhs);
    WorkerResult(WorkerResult &&rhs) noexcept;
    WorkerResult &operator=(WorkerResult &&rhs) noexcept;

    bool success() const;
    int error() const;
    QString errorString() const;

    static WorkerResult fail(int _error, const QString &_errorString);
    static WorkerResult pass();

private:
    explicit WorkerResult(std::unique_ptr<class WorkerResultPrivate> &&dptr);
    std::unique_ptr<class WorkerResultPrivate> d;
};

/**
 * @class WorkerBase
 * @brief Worker基类，提供Worker的基本功能
 */
class WorkerBase : public QObject {
    Q_OBJECT

public:
    enum MessageBoxType {
        QuestionYesNo = 1,
        WarningYesNo = 2,
        WarningContinueCancel = 3,
        WarningYesNoCancel = 4,
        Information = 5
    };

    WorkerBase(const QByteArray &protocol, const QByteArray &poolSocket, const QByteArray &appSocket);
    virtual ~WorkerBase();

    void dispatchLoop();
    void connectWorker(const QString &address);
    void disconnectWorker();

    void setMetaData(const QString &key, const QString &value);
    QString metaData(const QString &key) const;
    MetaData allMetaData() const;
    bool hasMetaData(const QString &key) const;
    QMap<QString, QVariant> mapConfig() const;

    bool configValue(const QString &key, bool defaultValue) const;
    int configValue(const QString &key, int defaultValue) const;
    QString configValue(const QString &key, const QString &defaultValue) const;

    void sendMetaData();
    void sendAndKeepMetaData();

    void sendData(const QByteArray &data);
    void dataReq();
    void workerStatus(const QString &host, bool connected);
    void canResume();

    void totalSize(qint64 _bytes);
    void processedSize(qint64 _bytes);
    void written(qint64 _bytes);
    void position(qint64 _pos);
    void truncated(qint64 _length);
    void speed(unsigned long _bytes_per_second);
    void redirection(const QUrl &_url);
    void mimeType(const QString &_type);
    void warning(const QString &_msg);
    void infoMessage(const QString &_msg);
    void statEntry(const QVariantMap &entry);
    void listEntry(const QVariantMap &entry);
    void listEntries(const QList<QVariantMap> &list);
    void exit();

    bool wasKilled() const;
    void lookupHost(const QString &host);
    void setIncomingMetaData(const MetaData &metaData);

Q_SIGNALS:
    /**
     * @brief 当数据准备好时发出的信号
     * @param data 数据内容
     */
    void data(const QByteArray &data);
    
    /**
     * @brief 当连接断开时发出的信号
     */
    void disconnected();
    
    /**
     * @brief 当操作完成时发出的信号
     */
    void finished();
    
    /**
     * @brief 当发生错误时发出的信号
     * @param code 错误代码
     * @param message 错误消息
     */
    void error(int code, const QString &message);

public: // Worker需要重写的虚函数
    virtual void appConnectionMade();
    virtual void setHost(QString const &host, quint16 port, QString const &user, QString const &pass);
    virtual WorkerResult openConnection();
    virtual void closeConnection();
    virtual WorkerResult stat(QUrl const &url);
    virtual WorkerResult put(QUrl const &url, int permissions, int flags);
    virtual WorkerResult special(const QByteArray &data);
    virtual WorkerResult listDir(QUrl const &url);
    virtual WorkerResult get(QUrl const &url);
    virtual WorkerResult open(QUrl const &url, QIODevice::OpenMode mode);
    virtual WorkerResult read(qint64 size);
    virtual WorkerResult write(const QByteArray &data);
    virtual WorkerResult seek(qint64 offset);
    virtual WorkerResult truncate(qint64 length);
    virtual WorkerResult close();
    virtual WorkerResult mimetype(QUrl const &url);
    virtual WorkerResult rename(QUrl const &src, QUrl const &dest, int flags);
    virtual WorkerResult symlink(QString const &target, QUrl const &dest, int flags);
    virtual WorkerResult copy(QUrl const &src, QUrl const &dest, int permissions, int flags);
    virtual WorkerResult del(QUrl const &url, bool isfile);
    virtual WorkerResult mkdir(QUrl const &url, int permissions);
    virtual WorkerResult chmod(QUrl const &url, int permissions);
    virtual WorkerResult setModificationTime(QUrl const &url, const QDateTime &mtime);

protected:
    virtual void worker_status();
    virtual void reparseConfiguration();
    virtual int openPasswordDialog(QVariantMap &info, const QString &errorMsg);
    virtual int messageBox(MessageBoxType type, const QString &text, const QString &title,
                         const QString &primaryActionText, const QString &secondaryActionText);
    virtual int messageBox(const QString &text, MessageBoxType type, const QString &title,
                         const QString &primaryActionText, const QString &secondaryActionText,
                         const QString &dontAskAgainName);

private:
    std::unique_ptr<WorkerBasePrivate> d;
};

} // namespace DFM

#endif // DFM_WORKERBASE_H 