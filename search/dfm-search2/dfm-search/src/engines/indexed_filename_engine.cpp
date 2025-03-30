#include <dfm-search/search_engine.h>
#include <dfm-search/index_engine.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QFileInfo>
#include <QFuture>
#include <QtConcurrent>
#include <QElapsedTimer>

namespace DFM {
namespace Search {

/**
 * @brief 基于索引的文件名搜索引擎
 * 
 * 使用预先建立的索引加速文件名搜索
 */
class IndexedFilenameEngine : public SearchEngine {
    Q_OBJECT
    
public:
    IndexedFilenameEngine();
    ~IndexedFilenameEngine() override;
    
    // 初始化索引
    bool initializeIndex(const IndexConfig& config);
    
    // 重写搜索方法
    bool search(const SearchQuery& query) override;
    bool cancel() override;
    
private:
    // 索引引擎
    std::shared_ptr<IndexEngine> indexEngine_;
    
    // 工作线程类
    class SearchWorker;
    std::unique_ptr<SearchWorker> worker_;
    
    // 内部方法
    void processResults(const QList<QString>& filePaths);
    
    // 搜索状态
    QMutex mutex_;
    bool searchRunning_;
};

/**
 * @brief 索引文件名搜索工作线程
 */
class IndexedFilenameEngine::SearchWorker {
public:
    SearchWorker(IndexedFilenameEngine* engine, const SearchQuery& query);
    ~SearchWorker();
    
    void start();
    void stop();
    
private:
    IndexedFilenameEngine* engine_;
    SearchQuery query_;
    QMutex mutex_;
    bool stopRequested_;
    QFuture<void> future_;
    
    void run();
    void searchInIndex();
};

// 实现IndexedFilenameEngine

IndexedFilenameEngine::IndexedFilenameEngine()
    : SearchEngine()
    , indexEngine_(nullptr)
    , worker_(nullptr)
    , searchRunning_(false)
{
}

IndexedFilenameEngine::~IndexedFilenameEngine()
{
    cancel();
}

bool IndexedFilenameEngine::initializeIndex(const IndexConfig& config)
{
    // 创建文件名索引引擎
    indexEngine_ = IndexEngineFactory::instance().createEngine(IndexType::Filename);
    if (!indexEngine_) {
        qWarning() << "无法创建文件名索引引擎";
        return false;
    }
    
    // 连接索引引擎的信号
    connect(indexEngine_.get(), &IndexEngine::progressChanged,
            this, [this](int progress) {
                emit progressChanged(progress);
            });
    
    connect(indexEngine_.get(), &IndexEngine::indexReady,
            this, [this]() {
                qDebug() << "索引准备就绪";
            });
    
    // 初始化索引
    return indexEngine_->initialize(config);
}

bool IndexedFilenameEngine::search(const SearchQuery& query)
{
    // 调用基类方法进行初始化
    if (!SearchEngine::search(query))
        return false;
    
    // 检查查询类型
    if (query.type != SearchType::Filename) {
        qWarning() << "IndexedFilenameEngine只支持文件名搜索";
        return false;
    }
    
    // 检查查询机制
    if (query.mechanism != SearchMechanism::Indexed) {
        qWarning() << "IndexedFilenameEngine只支持索引搜索";
        return false;
    }
    
    // 检查索引是否已初始化
    if (!indexEngine_ || !indexEngine_->isInitialized()) {
        qWarning() << "索引未初始化";
        return false;
    }
    
    // 确保没有正在运行的搜索
    cancel();
    
    // 创建并启动搜索工作线程
    worker_ = std::make_unique<SearchWorker>(this, query);
    worker_->start();
    
    {
        QMutexLocker locker(&mutex_);
        searchRunning_ = true;
    }
    
    return true;
}

bool IndexedFilenameEngine::cancel()
{
    {
        QMutexLocker locker(&mutex_);
        if (!searchRunning_)
            return false;
    }
    
    if (worker_) {
        worker_->stop();
        worker_.reset();
    }
    
    {
        QMutexLocker locker(&mutex_);
        searchRunning_ = false;
    }
    
    return SearchEngine::cancel();
}

void IndexedFilenameEngine::processResults(const QList<QString>& filePaths)
{
    if (filePaths.isEmpty())
        return;
    
    // 创建搜索结果
    SearchResult searchResult;
    
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists())
            continue;
        
        auto item = std::make_shared<FileResultItem>();
        
        // 填充文件信息
        item->displayName = fileInfo.fileName();
        item->uri = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
        item->lastModified = fileInfo.lastModified();
        item->filePath = fileInfo.absoluteFilePath();
        item->fileSize = fileInfo.size();
        item->fileType = fileInfo.suffix();
        item->isDirectory = fileInfo.isDir();
        
        // 添加到结果中
        searchResult.addItem(item);
    }
    
    // 返回结果
    d->addResults(searchResult);
}

// 实现SearchWorker

IndexedFilenameEngine::SearchWorker::SearchWorker(
    IndexedFilenameEngine* engine, const SearchQuery& query)
    : engine_(engine)
    , query_(query)
    , stopRequested_(false)
{
}

IndexedFilenameEngine::SearchWorker::~SearchWorker()
{
    stop();
}

void IndexedFilenameEngine::SearchWorker::start()
{
    // 使用QtConcurrent在线程池中执行
    future_ = QtConcurrent::run(&IndexedFilenameEngine::SearchWorker::run, this);
}

void IndexedFilenameEngine::SearchWorker::stop()
{
    {
        QMutexLocker locker(&mutex_);
        stopRequested_ = true;
    }
    
    if (future_.isRunning()) {
        future_.waitForFinished();
    }
}

void IndexedFilenameEngine::SearchWorker::run()
{
    QElapsedTimer timer;
    timer.start();
    
    engine_->d->setStatus(SearchStatus::Running);
    engine_->d->setProgress(0);
    
    // 在索引中搜索
    searchInIndex();
    
    // 检查是否被取消
    {
        QMutexLocker locker(&mutex_);
        if (stopRequested_) {
            engine_->d->setStatus(SearchStatus::Cancelled);
            return;
        }
    }
    
    // 搜索完成
    engine_->d->setProgress(100);
    engine_->d->setStatus(SearchStatus::Completed);
}

void IndexedFilenameEngine::SearchWorker::searchInIndex()
{
    // 这里应该是查询索引的实现
    // 由于我们没有实际的索引实现，这里只是作为示例
    
    // 模拟一些文件路径结果
    QList<QString> results;
    
    // 模拟进度
    for (int i = 0; i < 10; ++i) {
        {
            QMutexLocker locker(&mutex_);
            if (stopRequested_)
                break;
        }
        
        // 模拟一些延迟
        QThread::msleep(100);
        
        // 更新进度
        engine_->d->setProgress(i * 10);
    }
    
    // 在实际实现中，这里应该查询索引数据库
    // 并返回匹配的文件路径
    
    // 处理结果
    engine_->processResults(results);
}

// 注册搜索引擎工厂
class IndexedFilenameEngineFactory {
public:
    IndexedFilenameEngineFactory() {
        SearchEngineManager::instance().registerEngineCreator(
            SearchType::Filename,
            SearchMechanism::Indexed,
            []() -> std::shared_ptr<SearchEngine> {
                return std::make_shared<IndexedFilenameEngine>();
            }
        );
    }
};

// 全局静态实例，确保工厂在程序启动时注册
static IndexedFilenameEngineFactory factory;

} // namespace Search
} // namespace DFM

// 包含生成的moc文件
#include "indexed_filename_engine.moc"