#include "qsearch/index_manager.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <atomic>

namespace QSearch {

struct IndexInfo {
    IndexType type;
    IndexStatus status = IndexStatus::NotInitialized;
    QString statusMessage;
    QStringList paths;
    QDateTime lastUpdateTime;
    std::atomic<double> progress{0.0};
    QVariantMap config;
    QThread* workerThread = nullptr;
    QFileSystemWatcher* watcher = nullptr;
    
    // 索引统计
    qint64 fileCount = 0;
    qint64 indexSize = 0;
};

struct IndexManager::Impl {
    QMap<IndexType, IndexInfo*> indices;
    QMutex indexMutex;
    QString indexBasePath;
    
    Impl() {
        // 确定索引存储路径
        indexBasePath = QDir::homePath() + "/.cache/qsearch/indices";
        QDir().mkpath(indexBasePath);
        
        // 初始化每种索引类型
        indices[IndexType::Filename] = new IndexInfo{IndexType::Filename};
        indices[IndexType::FileContent] = new IndexInfo{IndexType::FileContent};
        indices[IndexType::Combined] = new IndexInfo{IndexType::Combined};
        indices[IndexType::Extended] = new IndexInfo{IndexType::Extended};
    }
    
    ~Impl() {
        QMutexLocker locker(&indexMutex);
        // 清理索引资源
        for (auto info : indices.values()) {
            if (info->workerThread) {
                info->workerThread->quit();
                info->workerThread->wait();
                delete info->workerThread;
            }
            if (info->watcher) {
                delete info->watcher;
            }
            delete info;
        }
    }
    
    QString getIndexPath(IndexType type) const {
        switch (type) {
            case IndexType::Filename: 
                return indexBasePath + "/filename";
            case IndexType::FileContent: 
                return indexBasePath + "/content";
            case IndexType::Combined: 
                return indexBasePath + "/combined";
            case IndexType::Extended: 
                return indexBasePath + "/extended";
            default:
                return indexBasePath + "/custom";
        }
    }
    
    bool initDatabase(IndexType type) {
        QString dbPath = getIndexPath(type) + "/index.db";
        QDir().mkpath(QFileInfo(dbPath).path());
        
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "qsearch_" + QString::number(static_cast<int>(type)));
        db.setDatabaseName(dbPath);
        
        if (!db.open()) {
            return false;
        }
        
        QSqlQuery query(db);
        
        // 根据索引类型创建不同的表结构
        switch (type) {
            case IndexType::Filename:
                query.exec("CREATE TABLE IF NOT EXISTS files ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT NOT NULL,"
                          "path TEXT NOT NULL UNIQUE,"
                          "size INTEGER,"
                          "modified_time INTEGER,"
                          "is_directory INTEGER)");
                query.exec("CREATE INDEX IF NOT EXISTS idx_filename ON files(name)");
                query.exec("CREATE INDEX IF NOT EXISTS idx_filepath ON files(path)");
                break;
                
            case IndexType::FileContent:
                query.exec("CREATE TABLE IF NOT EXISTS files ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "path TEXT NOT NULL UNIQUE,"
                          "modified_time INTEGER)");
                query.exec("CREATE TABLE IF NOT EXISTS content_index ("
                          "file_id INTEGER,"
                          "term TEXT,"
                          "positions TEXT,"
                          "FOREIGN KEY(file_id) REFERENCES files(id))");
                query.exec("CREATE INDEX IF NOT EXISTS idx_filepath ON files(path)");
                query.exec("CREATE INDEX IF NOT EXISTS idx_term ON content_index(term)");
                break;
                
            case IndexType::Combined:
                // 组合索引可以重用上面两种索引，或者创建自己的表结构
                query.exec("CREATE TABLE IF NOT EXISTS files ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT NOT NULL,"
                          "path TEXT NOT NULL UNIQUE,"
                          "size INTEGER,"
                          "modified_time INTEGER,"
                          "is_directory INTEGER)");
                query.exec("CREATE TABLE IF NOT EXISTS content_index ("
                          "file_id INTEGER,"
                          "term TEXT,"
                          "positions TEXT,"
                          "FOREIGN KEY(file_id) REFERENCES files(id))");
                query.exec("CREATE INDEX IF NOT EXISTS idx_filename ON files(name)");
                query.exec("CREATE INDEX IF NOT EXISTS idx_filepath ON files(path)");
                query.exec("CREATE INDEX IF NOT EXISTS idx_term ON content_index(term)");
                break;
                
            default:
                break;
        }
        
        return true;
    }
};

IndexManager::IndexManager(QObject* parent) : QObject(parent), d(new Impl) {
}

IndexManager::~IndexManager() {
}

bool IndexManager::initIndex(IndexType type, const QStringList& paths, const QVariantMap& options) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    
    // 如果已经在初始化或更新中，不能再次初始化
    if (info->status == IndexStatus::Initializing || 
        info->status == IndexStatus::Updating ||
        info->status == IndexStatus::Rebuilding) {
        return false;
    }
    
    // 设置初始化状态
    info->status = IndexStatus::Initializing;
    info->statusMessage = "正在初始化索引...";
    info->paths = paths;
    info->progress = 0.0;
    
    // 将选项保存到配置中
    for (auto it = options.begin(); it != options.end(); ++it) {
        info->config[it.key()] = it.value();
    }
    
    emit indexStatusChanged(type, info->status);
    emit indexProgressChanged(type, info->progress);
    
    // 初始化索引数据库
    if (!d->initDatabase(type)) {
        info->status = IndexStatus::Error;
        info->statusMessage = "索引数据库初始化失败";
        emit indexStatusChanged(type, info->status);
        emit indexError(type, info->statusMessage);
        return false;
    }
    
    // 创建工作线程进行索引构建
    if (info->workerThread) {
        info->workerThread->quit();
        info->workerThread->wait();
        delete info->workerThread;
    }
    
    info->workerThread = new QThread();
    
    // 创建索引构建对象并移动到工作线程
    auto indexBuilder = new QObject(); // 这里应该是自定义的索引构建类
    indexBuilder->moveToThread(info->workerThread);
    
    // 连接信号
    connect(info->workerThread, &QThread::started, indexBuilder, [this, type, info, indexBuilder]() {
        // 实现索引构建逻辑
        // 这里是索引构建的伪代码，实际实现应该更复杂
        
        // 扫描目录，创建索引
        for (int i = 0; i < info->paths.size(); ++i) {
            QString path = info->paths[i];
            
            // 更新进度
            info->progress = static_cast<double>(i) / info->paths.size();
            emit indexProgressChanged(type, info->progress);
            
            // 示例进度模拟，实际应根据文件扫描进度更新
            QThread::msleep(500); // 模拟耗时操作
        }
        
        // 索引完成
        info->status = IndexStatus::Ready;
        info->statusMessage = "索引就绪";
        info->progress = 1.0;
        info->lastUpdateTime = QDateTime::currentDateTime();
        
        // 更新索引统计信息（这里仅为示例）
        info->fileCount = 1000; // 实际应该是真实的文件计数
        info->indexSize = 1024 * 1024; // 实际应该是真实的索引大小
        
        // 设置文件系统监控
        if (!info->watcher) {
            info->watcher = new QFileSystemWatcher();
            connect(info->watcher, &QFileSystemWatcher::directoryChanged, 
                    this, [this, type](const QString& path) {
                // 目录变更，调度更新索引
                QTimer::singleShot(5000, this, [this, type]() {
                    updateIndex(type);
                });
            });
        }
        
        // 添加监控路径
        for (const QString& path : info->paths) {
            info->watcher->addPath(path);
            
            // 对于大目录，可能需要递归添加子目录
            QDir dir(path);
            QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& subdir : subdirs) {
                info->watcher->addPath(path + "/" + subdir);
            }
        }
        
        emit indexStatusChanged(type, info->status);
        emit indexProgressChanged(type, info->progress);
        emit indexCompleted(type);
        
        // 清理资源
        indexBuilder->deleteLater();
    });
    
    info->workerThread->start();
    
    return true;
}

bool IndexManager::updateIndex(IndexType type) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    
    // 只有Ready状态的索引才能更新
    if (info->status != IndexStatus::Ready) {
        return false;
    }
    
    info->status = IndexStatus::Updating;
    info->statusMessage = "正在更新索引...";
    info->progress = 0.0;
    
    emit indexStatusChanged(type, info->status);
    emit indexProgressChanged(type, info->progress);
    
    // 这里应该实现增量更新索引的逻辑
    // 简化起见，这里仅模拟一个延迟操作
    QTimer::singleShot(3000, this, [this, type, info]() {
        info->status = IndexStatus::Ready;
        info->statusMessage = "索引更新完成";
        info->progress = 1.0;
        info->lastUpdateTime = QDateTime::currentDateTime();
        
        emit indexStatusChanged(type, info->status);
        emit indexProgressChanged(type, info->progress);
        emit indexCompleted(type);
    });
    
    return true;
}

bool IndexManager::rebuildIndex(IndexType type) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    QStringList paths = info->paths;
    QVariantMap options = info->config;
    
    // 清除索引
    clearIndex(type);
    
    // 重新初始化
    return initIndex(type, paths, options);
}

bool IndexManager::stopIndexing(IndexType type) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    
    // 只有正在进行的索引操作才能停止
    if (info->status != IndexStatus::Initializing && 
        info->status != IndexStatus::Updating && 
        info->status != IndexStatus::Rebuilding) {
        return false;
    }
    
    if (info->workerThread) {
        info->workerThread->quit();
        info->workerThread->wait();
        delete info->workerThread;
        info->workerThread = nullptr;
    }
    
    info->status = IndexStatus::Error;
    info->statusMessage = "索引操作被用户中止";
    
    emit indexStatusChanged(type, info->status);
    emit indexError(type, info->statusMessage);
    
    return true;
}

void IndexManager::clearIndex(IndexType type) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return;
    }
    
    auto info = d->indices[type];
    
    // 停止任何正在进行的索引操作
    if (info->workerThread) {
        info->workerThread->quit();
        info->workerThread->wait();
        delete info->workerThread;
        info->workerThread = nullptr;
    }
    
    // 移除文件监控
    if (info->watcher) {
        delete info->watcher;
        info->watcher = nullptr;
    }
    
    // 删除索引文件
    QString indexPath = d->getIndexPath(type);
    QDir dir(indexPath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    
    // 重置索引状态
    info->status = IndexStatus::NotInitialized;
    info->statusMessage = "索引已清除";
    info->progress = 0.0;
    info->fileCount = 0;
    info->indexSize = 0;
    
    emit indexStatusChanged(type, info->status);
}

IndexStatus IndexManager::status(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    return d->indices.contains(type) ? d->indices[type]->status : IndexStatus::Error;
}

QString IndexManager::statusMessage(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    return d->indices.contains(type) ? d->indices[type]->statusMessage : "未知索引类型";
}

double IndexManager::indexProgress(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    return d->indices.contains(type) ? d->indices[type]->progress : 0.0;
}

bool IndexManager::setIndexConfig(IndexType type, const QString& key, const QVariant& value) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    d->indices[type]->config[key] = value;
    return true;
}

QVariant IndexManager::indexConfig(IndexType type, const QString& key) const {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return QVariant();
    }
    
    return d->indices[type]->config.value(key);
}

bool IndexManager::addIndexPath(IndexType type, const QString& path) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    
    // 检查路径是否已存在
    if (info->paths.contains(path)) {
        return true; // 已存在，视为成功
    }
    
    // 添加路径
    info->paths.append(path);
    
    // 如果索引已就绪，添加监控并调度更新
    if (info->status == IndexStatus::Ready && info->watcher) {
        info->watcher->addPath(path);
        
        // 调度增量更新
        QTimer::singleShot(1000, this, [this, type]() {
            updateIndex(type);
        });
    }
    
    return true;
}

bool IndexManager::removeIndexPath(IndexType type, const QString& path) {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return false;
    }
    
    auto info = d->indices[type];
    
    // 检查路径是否存在
    if (!info->paths.contains(path)) {
        return false;
    }
    
    // 移除路径
    info->paths.removeAll(path);
    
    // 如果索引已就绪，移除监控并调度重建
    if (info->status == IndexStatus::Ready && info->watcher) {
        info->watcher->removePath(path);
        
        // 调度重建索引
        QTimer::singleShot(1000, this, [this, type]() {
            rebuildIndex(type);
        });
    }
    
    return true;
}

QStringList IndexManager::indexPaths(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return QStringList();
    }
    
    return d->indices[type]->paths;
}

qint64 IndexManager::indexedFileCount(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return 0;
    }
    
    return d->indices[type]->fileCount;
}

qint64 IndexManager::indexSize(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return 0;
    }
    
    return d->indices[type]->indexSize;
}

QDateTime IndexManager::lastUpdateTime(IndexType type) const {
    QMutexLocker locker(&d->indexMutex);
    
    if (!d->indices.contains(type)) {
        return QDateTime();
    }
    
    return d->indices[type]->lastUpdateTime;
}

} // namespace QSearch 