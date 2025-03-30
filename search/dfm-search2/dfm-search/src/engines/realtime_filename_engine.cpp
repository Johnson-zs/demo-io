#include <dfm-search/search_engine.h>
#include <dfm-search/search_filter.h>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFuture>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QtConcurrent>
#include <QElapsedTimer>

namespace DFM {
namespace Search {

/**
 * @brief 实时文件名搜索引擎类
 * 
 * 通过实时遍历文件系统查找匹配的文件名
 */
class RealtimeFilenameEngine : public SearchEngine {
    Q_OBJECT
    
public:
    RealtimeFilenameEngine();
    ~RealtimeFilenameEngine() override;
    
    // 重写搜索方法以实现实际的搜索逻辑
    bool search(const SearchQuery& query) override;
    bool cancel() override;
    
private:
    // 内部类定义，处理搜索线程
    class SearchWorker : public QThread {
    public:
        SearchWorker(RealtimeFilenameEngine* engine, const SearchQuery& query);
        ~SearchWorker() override;
        
        void run() override;
        void stop();
        
    private:
        RealtimeFilenameEngine* engine_;
        SearchQuery query_;
        QMutex mutex_;
        bool stopRequested_;
        
        void searchInPath(const QString& path);
        bool matchesFilename(const QString& filename);
    };
    
    // 搜索工作线程
    QScopedPointer<SearchWorker> worker_;
    
    // 辅助方法
    void processResults(const QList<QFileInfo>& results);
    
    friend class SearchWorker;
};

// 实现RealtimeFilenameEngine

RealtimeFilenameEngine::RealtimeFilenameEngine()
    : SearchEngine()
    , worker_(nullptr)
{
}

RealtimeFilenameEngine::~RealtimeFilenameEngine()
{
    cancel();
}

bool RealtimeFilenameEngine::search(const SearchQuery& query)
{
    // 调用基类方法进行初始化
    if (!SearchEngine::search(query))
        return false;
    
    // 检查查询类型
    if (query.type != SearchType::Filename) {
        qWarning() << "RealtimeFilenameEngine只支持文件名搜索";
        return false;
    }
    
    // 检查查询机制
    if (query.mechanism != SearchMechanism::Realtime) {
        qWarning() << "RealtimeFilenameEngine只支持实时搜索";
        return false;
    }
    
    // 检查搜索路径
    if (query.options.searchPaths.isEmpty()) {
        qWarning() << "搜索路径不能为空";
        return false;
    }
    
    // 创建并启动搜索工作线程
    worker_.reset(new SearchWorker(this, query));
    worker_->start();
    
    return true;
}

bool RealtimeFilenameEngine::cancel()
{
    if (worker_) {
        worker_->stop();
        worker_->wait(); // 等待线程结束
        worker_.reset();
    }
    
    return SearchEngine::cancel();
}

void RealtimeFilenameEngine::processResults(const QList<QFileInfo>& results)
{
    if (results.isEmpty())
        return;
    
    // 创建搜索结果
    SearchResult searchResult;
    
    for (const QFileInfo& fileInfo : results) {
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

RealtimeFilenameEngine::SearchWorker::SearchWorker(
    RealtimeFilenameEngine* engine, const SearchQuery& query)
    : engine_(engine)
    , query_(query)
    , stopRequested_(false)
{
}

RealtimeFilenameEngine::SearchWorker::~SearchWorker()
{
    stop();
    wait();
}

void RealtimeFilenameEngine::SearchWorker::run()
{
    QElapsedTimer timer;
    timer.start();
    
    int totalPaths = query_.options.searchPaths.size();
    int completedPaths = 0;
    
    // 遍历每个搜索路径
    for (const QString& path : query_.options.searchPaths) {
        {
            QMutexLocker locker(&mutex_);
            if (stopRequested_)
                break;
        }
        
        searchInPath(path);
        
        completedPaths++;
        int progress = (completedPaths * 100) / totalPaths;
        engine_->d->setProgress(progress);
    }
    
    // 搜索完成
    if (!stopRequested_) {
        engine_->d->setStatus(SearchStatus::Completed);
    }
}

void RealtimeFilenameEngine::SearchWorker::stop()
{
    QMutexLocker locker(&mutex_);
    stopRequested_ = true;
}

void RealtimeFilenameEngine::SearchWorker::searchInPath(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    
    // 创建过滤器
    auto filter = FilterFactory::createFromOptions(query_.options);
    
    // 文件名匹配过滤器
    auto filenameFilter = FilterFactory::createFilenameFilter(
        query_.queryString, query_.options.caseSensitive);
    
    // 创建一个AND过滤器组合
    auto filterGroup = FilterFactory::createFilterGroup();
    filterGroup->addFilter(filter);
    filterGroup->addFilter(filenameFilter);
    
    // 遍历目录内容
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, flags);
    
    QList<QFileInfo> batchResults;
    const int batchSize = 100; // 每批处理的文件数量
    
    while (it.hasNext()) {
        {
            QMutexLocker locker(&mutex_);
            if (stopRequested_)
                break;
        }
        
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        // 应用过滤器
        if (filterGroup->matches(fileInfo)) {
            batchResults.append(fileInfo);
            
            // 达到批处理大小时发送结果
            if (batchResults.size() >= batchSize) {
                engine_->processResults(batchResults);
                batchResults.clear();
            }
        }
    }
    
    // 处理剩余的结果
    if (!batchResults.isEmpty()) {
        engine_->processResults(batchResults);
    }
}

// 注册搜索引擎工厂
class RealtimeFilenameEngineFactory {
public:
    RealtimeFilenameEngineFactory() {
        SearchEngineManager::instance().registerEngineCreator(
            SearchType::Filename,
            SearchMechanism::Realtime,
            []() -> std::shared_ptr<SearchEngine> {
                return std::make_shared<RealtimeFilenameEngine>();
            }
        );
    }
};

// 全局静态实例，确保工厂在程序启动时注册
static RealtimeFilenameEngineFactory factory;

} // namespace Search
} // namespace DFM

// 包含生成的moc文件
#include "realtime_filename_engine.moc" 