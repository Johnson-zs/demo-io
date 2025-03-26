#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include "../search/searchquery.h"
#include "../search/searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 远程/分布式搜索连接器接口
class QTSEARCHKIT_EXPORT SearchConnector : public QObject {
    Q_OBJECT
public:
    enum ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    
    explicit SearchConnector(QObject* parent = nullptr);
    virtual ~SearchConnector() override;
    
    // 连接管理
    virtual bool connect(const QUrl& endpoint, const QVariantMap& credentials = QVariantMap()) = 0;
    virtual void disconnect() = 0;
    virtual ConnectionStatus status() const = 0;
    
    // 元数据
    virtual QString connectorType() const = 0;
    virtual QString displayName() const = 0;
    virtual SearchFeatures remoteFeatures() const = 0;
    
    // 远程搜索
    virtual QFuture<std::shared_ptr<SearchResultSet>> search(
        const SearchQuery& query, 
        SearchType type) = 0;
    
    // 健康检查
    virtual bool ping() = 0;
    virtual QVariantMap statistics() = 0;
    
signals:
    void connectionStatusChanged(ConnectionStatus status);
    void connectionError(const QString& error);
    void searchComplete(const QString& queryId);
    void searchError(const QString& queryId, const QString& error);
    
protected:
    // 工具方法
    void emitStatusChanged(ConnectionStatus status);
    void emitConnectionError(const QString& error);
};

// HTTP连接器示例（其他实现可能包括dbus、grpc等）
class QTSEARCHKIT_EXPORT HttpSearchConnector : public SearchConnector {
    Q_OBJECT
public:
    explicit HttpSearchConnector(QObject* parent = nullptr);
    
    bool connect(const QUrl& endpoint, const QVariantMap& credentials = QVariantMap()) override;
    void disconnect() override;
    ConnectionStatus status() const override;
    
    QString connectorType() const override;
    QString displayName() const override;
    SearchFeatures remoteFeatures() const override;
    
    QFuture<std::shared_ptr<SearchResultSet>> search(
        const SearchQuery& query, 
        SearchType type) override;
    
    bool ping() override;
    QVariantMap statistics() override;
    
    // HTTP特有配置
    void setTimeout(int msec);
    int timeout() const;
    
    void setHeaders(const QMap<QString, QString>& headers);
    QMap<QString, QString> headers() const;
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 