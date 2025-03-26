#pragma once

#include <QObject>
#include <QFuture>
#include <QString>
#include <QStringList>
#include "indexconfig.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT IndexManager : public QObject {
    Q_OBJECT
public:
    explicit IndexManager(QObject* parent = nullptr);
    ~IndexManager() override;
    
    // 索引配置
    void setConfiguration(const IndexConfig& config);
    IndexConfig configuration() const;
    
    // 索引操作
    QFuture<bool> createIndex();
    QFuture<bool> updateIndex();
    bool pauseIndexing();
    bool resumeIndexing();
    bool stopIndexing();
    QFuture<bool> resetIndex();
    
    // 索引状态
    IndexStatus status() const;
    QString statusMessage() const;
    int indexingProgress() const; // 0-100
    
    // 索引统计
    QFuture<QVariantMap> statistics();
    
signals:
    void indexingStarted();
    void indexingFinished();
    void indexingPaused();
    void indexingResumed();
    void indexingStopped();
    void indexingReset();
    void indexingError(const QString& error);
    void statusChanged(IndexStatus status);
    void progressChanged(int progress);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 