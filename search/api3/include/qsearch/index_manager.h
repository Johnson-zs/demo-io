#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include "global.h"

namespace QSearch {

enum class IndexType {
    Filename,       // 文件名索引
    FileContent,    // 文件内容索引
    Combined,       // 组合索引
    Extended        // 扩展索引类型
};

enum class IndexStatus {
    NotInitialized, // 未初始化
    Initializing,   // 正在初始化
    Ready,          // 准备就绪
    Updating,       // 更新中
    Rebuilding,     // 重建中
    Error           // 错误状态
};

class QSEARCH_EXPORT IndexManager : public QObject {
    Q_OBJECT
public:
    explicit IndexManager(QObject* parent = nullptr);
    ~IndexManager();
    
    // 索引管理
    bool initIndex(IndexType type, const QStringList& paths, const QVariantMap& options = {});
    bool updateIndex(IndexType type);
    bool rebuildIndex(IndexType type);
    bool stopIndexing(IndexType type);
    void clearIndex(IndexType type);
    
    // 索引状态
    IndexStatus status(IndexType type) const;
    QString statusMessage(IndexType type) const;
    double indexProgress(IndexType type) const;
    
    // 索引配置
    bool setIndexConfig(IndexType type, const QString& key, const QVariant& value);
    QVariant indexConfig(IndexType type, const QString& key) const;
    
    // 索引路径管理
    bool addIndexPath(IndexType type, const QString& path);
    bool removeIndexPath(IndexType type, const QString& path);
    QStringList indexPaths(IndexType type) const;
    
    // 索引统计
    qint64 indexedFileCount(IndexType type) const;
    qint64 indexSize(IndexType type) const;
    QDateTime lastUpdateTime(IndexType type) const;
    
signals:
    void indexStatusChanged(IndexType type, IndexStatus status);
    void indexProgressChanged(IndexType type, double progress);
    void indexError(IndexType type, const QString& errorMessage);
    void indexCompleted(IndexType type);
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 