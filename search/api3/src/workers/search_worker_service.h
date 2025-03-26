#pragma once

#include <QObject>
#include "../../include/qsearch/ipc/ipc_channel.h"

namespace QSearch {

class SearchWorkerService : public QObject {
    Q_OBJECT
public:
    explicit SearchWorkerService(QObject* parent = nullptr);
    ~SearchWorkerService();
    
    bool start();
    void stop();
    
private slots:
    void handleRequest(const QString& method, const QJsonObject& params, QJsonObject& response);
    void handleStartSearch(const QJsonObject& params, QJsonObject& response);
    void handleStopSearch(const QJsonObject& params, QJsonObject& response);
    // ... 其他处理方法 ...
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 