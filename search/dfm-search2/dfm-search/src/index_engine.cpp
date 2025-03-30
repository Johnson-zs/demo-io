#include <dfm-search/index_engine.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QFile>
#include <QDateTime>

namespace DFM {
namespace Search {

// IndexEngine私有实现
class IndexEngine::Impl {
public:
    Impl(IndexEngine* q)
        : q_ptr(q)
        , status(IndexStatus::Uninitialized)
        , progressPercentage(0)
    {
    }
    
    // 引用外部类
    IndexEngine* q_ptr;
    
    // 状态相关
    IndexStatus status;
    int progressPercentage;
    IndexConfig config;
    QDateTime lastUpdateTime;
    qint64 indexSize{0};
    int itemCount{0};
    
    // 互斥锁
    mutable QMutex mutex;
    
    // 回调函数
    ProgressCallback progressCallback;
    StatusCallback statusCallback;
    
    // 设置状态
    void setStatus(IndexStatus newStatus) {
        QMutexLocker locker(&mutex);
        if (status != newStatus) {
            status = newStatus;
            
            // 调用回调
            if (statusCallback) {
                statusCallback(status);
            }
            
            // 发送信号
            emit q_ptr->statusChanged(status);
            
            // 特殊状态处理
            if (status == IndexStatus::Ready) {
                emit q_ptr->indexReady();
            } else if (status == IndexStatus::Error) {
                emit q_ptr->indexError("索引操作过程中发生错误");
            }
        }
    }
    
    // 设置进度
    void setProgress(int percentage) {
        QMutexLocker locker(&mutex);
        if (progressPercentage != percentage) {
            progressPercentage = percentage;
            
            // 调用回调
            if (progressCallback) {
                progressCallback(progressPercentage);
            }
            
            // 发送信号
            emit q_ptr->progressChanged(progressPercentage);
        }
    }
};

// 构造函数
IndexEngine::IndexEngine()
    : d(new Impl(this))
{
}

// 析构函数
IndexEngine::~IndexEngine() = default;

// 初始化索引
bool IndexEngine::initialize(const IndexConfig& config)
{
    if (d->status != IndexStatus::Uninitialized) {
        qWarning() << "索引引擎已经初始化";
        return false;
    }
    
    // 保存配置
    d->config = config;
    
    // 更新状态
    d->setStatus(IndexStatus::Initializing);
    d->setProgress(0);
    
    // 模拟初始化流程
    // 实际实现中应该异步处理
    QDir dir;
    if (!d->config.indexStoragePath.isEmpty() && !dir.exists(d->config.indexStoragePath)) {
        if (!dir.mkpath(d->config.indexStoragePath)) {
            qWarning() << "无法创建索引存储目录:" << d->config.indexStoragePath;
            d->setStatus(IndexStatus::Error);
            return false;
        }
    }
    
    // 假设初始化成功
    d->lastUpdateTime = QDateTime::currentDateTime();
    d->setStatus(IndexStatus::Ready);
    d->setProgress(100);
    
    return true;
}

// 更新索引
bool IndexEngine::update(const QStringList& paths)
{
    if (d->status == IndexStatus::Uninitialized) {
        qWarning() << "索引引擎未初始化";
        return false;
    }
    
    if (d->status == IndexStatus::Updating) {
        qWarning() << "索引引擎正在更新中";
        return false;
    }
    
    // 更新状态
    d->setStatus(IndexStatus::Updating);
    d->setProgress(0);
    
    // 待更新的路径
    QStringList pathsToUpdate = paths;
    if (pathsToUpdate.isEmpty()) {
        pathsToUpdate = d->config.indexPaths;
    }
    
    // 模拟更新进度
    // 实际实现中应该异步处理
    d->setProgress(50);
    
    // 假设更新成功
    d->lastUpdateTime = QDateTime::currentDateTime();
    d->itemCount += 10; // 假设增加了10个项目
    d->indexSize += 1024 * 1024; // 假设增加了1MB索引大小
    
    d->setStatus(IndexStatus::Ready);
    d->setProgress(100);
    
    return true;
}

// 重建索引
bool IndexEngine::rebuild()
{
    if (d->status == IndexStatus::Uninitialized) {
        qWarning() << "索引引擎未初始化";
        return false;
    }
    
    if (d->status == IndexStatus::Updating) {
        qWarning() << "索引引擎正在更新中";
        return false;
    }
    
    // 先清空索引
    if (!clear()) {
        return false;
    }
    
    // 然后更新全部路径
    return update();
}

// 清空索引
bool IndexEngine::clear()
{
    if (d->status == IndexStatus::Uninitialized) {
        qWarning() << "索引引擎未初始化";
        return false;
    }
    
    if (d->status == IndexStatus::Updating) {
        qWarning() << "索引引擎正在更新中";
        return false;
    }
    
    // 更新状态
    d->setStatus(IndexStatus::Updating);
    d->setProgress(0);
    
    // 模拟清空索引
    // 实际实现中应该异步处理
    if (!d->config.indexStoragePath.isEmpty()) {
        // 删除索引文件的操作
    }
    
    d->itemCount = 0;
    d->indexSize = 0;
    
    d->setStatus(IndexStatus::Ready);
    d->setProgress(100);
    
    return true;
}

// 是否已初始化
bool IndexEngine::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->status != IndexStatus::Uninitialized;
}

// 获取状态
IndexStatus IndexEngine::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

// 获取进度
int IndexEngine::progressPercentage() const
{
    QMutexLocker locker(&d->mutex);
    return d->progressPercentage;
}

// 获取最后更新时间
QDateTime IndexEngine::lastUpdateTime() const
{
    QMutexLocker locker(&d->mutex);
    return d->lastUpdateTime;
}

// 获取索引大小
qint64 IndexEngine::indexSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->indexSize;
}

// 获取索引项数量
int IndexEngine::itemCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->itemCount;
}

// 设置进度回调
void IndexEngine::setProgressCallback(ProgressCallback callback)
{
    QMutexLocker locker(&d->mutex);
    d->progressCallback = std::move(callback);
}

// 设置状态回调
void IndexEngine::setStatusCallback(StatusCallback callback)
{
    QMutexLocker locker(&d->mutex);
    d->statusCallback = std::move(callback);
}

// IndexEngineFactory实现
IndexEngineFactory& IndexEngineFactory::instance()
{
    static IndexEngineFactory instance;
    return instance;
}

std::shared_ptr<IndexEngine> IndexEngineFactory::createEngine(IndexType type)
{
    // 简单实现，直接返回基类IndexEngine的实例
    // 实际应用中应该根据类型返回不同的派生类
    return std::make_shared<IndexEngine>();
}

} // namespace Search
} // namespace DFM 