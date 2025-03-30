#include <dfm-search/search_engine.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadPool>

namespace DFM {
namespace Search {

// SearchResultItem 实现
class SearchResultItem::Impl {
public:
    QString displayName;
    QUrl uri;
    QDateTime lastModified;
    double relevanceScore{0.0};
    QVariantMap metadata;
};

SearchResultItem::SearchResultItem()
    : d(new Impl)
{
}

SearchResultItem::~SearchResultItem() = default;

QString SearchResultItem::displayName() const
{
    return d->displayName;
}

void SearchResultItem::setDisplayName(const QString& name)
{
    d->displayName = name;
}

QUrl SearchResultItem::uri() const
{
    return d->uri;
}

void SearchResultItem::setUri(const QUrl& uri)
{
    d->uri = uri;
}

QDateTime SearchResultItem::lastModified() const
{
    return d->lastModified;
}

void SearchResultItem::setLastModified(const QDateTime& dateTime)
{
    d->lastModified = dateTime;
}

double SearchResultItem::relevanceScore() const
{
    return d->relevanceScore;
}

void SearchResultItem::setRelevanceScore(double score)
{
    d->relevanceScore = score;
}

QVariantMap SearchResultItem::metadata() const
{
    return d->metadata;
}

void SearchResultItem::setMetadata(const QVariantMap& meta)
{
    d->metadata = meta;
}

void SearchResultItem::addMetadata(const QString& key, const QVariant& value)
{
    d->metadata.insert(key, value);
}

// FileResultItem 实现
class FileResultItem::Impl {
public:
    QString filePath;
    qint64 fileSize{0};
    QString fileType;
    bool isDirectory{false};
};

FileResultItem::FileResultItem()
    : d(new Impl)
{
}

FileResultItem::~FileResultItem() = default;

QString FileResultItem::filePath() const
{
    return d->filePath;
}

void FileResultItem::setFilePath(const QString& path)
{
    d->filePath = path;
}

qint64 FileResultItem::fileSize() const
{
    return d->fileSize;
}

void FileResultItem::setFileSize(qint64 size)
{
    d->fileSize = size;
}

QString FileResultItem::fileType() const
{
    return d->fileType;
}

void FileResultItem::setFileType(const QString& type)
{
    d->fileType = type;
}

bool FileResultItem::isDirectory() const
{
    return d->isDirectory;
}

void FileResultItem::setIsDirectory(bool isDir)
{
    d->isDirectory = isDir;
}

// ContentResultItem 实现
class ContentResultItem::Impl {
public:
    QString matchedContent;
    int lineNumber{0};
    int charPosition{0};
};

ContentResultItem::ContentResultItem()
    : d(new Impl)
{
}

ContentResultItem::~ContentResultItem() = default;

QString ContentResultItem::matchedContent() const
{
    return d->matchedContent;
}

void ContentResultItem::setMatchedContent(const QString& content)
{
    d->matchedContent = content;
}

int ContentResultItem::lineNumber() const
{
    return d->lineNumber;
}

void ContentResultItem::setLineNumber(int line)
{
    d->lineNumber = line;
}

int ContentResultItem::charPosition() const
{
    return d->charPosition;
}

void ContentResultItem::setCharPosition(int pos)
{
    d->charPosition = pos;
}

// AppResultItem 实现
class AppResultItem::Impl {
public:
    QString appId;
    QString desktopFilePath;
    QString iconName;
    QString execCommand;
};

AppResultItem::AppResultItem()
    : d(new Impl)
{
}

AppResultItem::~AppResultItem() = default;

QString AppResultItem::appId() const
{
    return d->appId;
}

void AppResultItem::setAppId(const QString& id)
{
    d->appId = id;
}

QString AppResultItem::desktopFilePath() const
{
    return d->desktopFilePath;
}

void AppResultItem::setDesktopFilePath(const QString& path)
{
    d->desktopFilePath = path;
}

QString AppResultItem::iconName() const
{
    return d->iconName;
}

void AppResultItem::setIconName(const QString& name)
{
    d->iconName = name;
}

QString AppResultItem::execCommand() const
{
    return d->execCommand;
}

void AppResultItem::setExecCommand(const QString& cmd)
{
    d->execCommand = cmd;
}

// ImageResultItem 实现
class ImageResultItem::Impl {
public:
    QString ocrText;
    QRect textRegion;
};

ImageResultItem::ImageResultItem()
    : d(new Impl)
{
}

ImageResultItem::~ImageResultItem() = default;

QString ImageResultItem::ocrText() const
{
    return d->ocrText;
}

void ImageResultItem::setOcrText(const QString& text)
{
    d->ocrText = text;
}

QRect ImageResultItem::textRegion() const
{
    return d->textRegion;
}

void ImageResultItem::setTextRegion(const QRect& region)
{
    d->textRegion = region;
}

// SearchOptions 实现
class SearchOptions::Impl {
public:
    bool caseSensitive{false};
    bool useRegex{false};
    bool fuzzyMatch{false};
    bool pinyinMatch{false};
    int maxResults{100};
    int pageSize{20};
    std::chrono::milliseconds timeout{3000};
    QVariantMap customOptions;
    QStringList searchPaths;
    QStringList excludePaths;
    QStringList fileFilters;
};

SearchOptions::SearchOptions()
    : d(new Impl)
{
}

SearchOptions::~SearchOptions() = default;

bool SearchOptions::caseSensitive() const
{
    return d->caseSensitive;
}

bool SearchOptions::useRegex() const
{
    return d->useRegex;
}

bool SearchOptions::fuzzyMatch() const
{
    return d->fuzzyMatch;
}

bool SearchOptions::pinyinMatch() const
{
    return d->pinyinMatch;
}

int SearchOptions::maxResults() const
{
    return d->maxResults;
}

int SearchOptions::pageSize() const
{
    return d->pageSize;
}

std::chrono::milliseconds SearchOptions::timeout() const
{
    return d->timeout;
}

const QVariantMap& SearchOptions::customOptions() const
{
    return d->customOptions;
}

QVariantMap& SearchOptions::customOptions()
{
    return d->customOptions;
}

const QStringList& SearchOptions::searchPaths() const
{
    return d->searchPaths;
}

QStringList& SearchOptions::searchPaths()
{
    return d->searchPaths;
}

const QStringList& SearchOptions::excludePaths() const
{
    return d->excludePaths;
}

QStringList& SearchOptions::excludePaths()
{
    return d->excludePaths;
}

const QStringList& SearchOptions::fileFilters() const
{
    return d->fileFilters;
}

QStringList& SearchOptions::fileFilters()
{
    return d->fileFilters;
}

void SearchOptions::setCaseSensitive(bool sensitive)
{
    d->caseSensitive = sensitive;
}

void SearchOptions::setUseRegex(bool useRegex)
{
    d->useRegex = useRegex;
}

void SearchOptions::setFuzzyMatch(bool fuzzy)
{
    d->fuzzyMatch = fuzzy;
}

void SearchOptions::setPinyinMatch(bool pinyin)
{
    d->pinyinMatch = pinyin;
}

void SearchOptions::setMaxResults(int maxResults)
{
    d->maxResults = maxResults;
}

void SearchOptions::setPageSize(int pageSize)
{
    d->pageSize = pageSize;
}

void SearchOptions::setTimeout(std::chrono::milliseconds timeout)
{
    d->timeout = timeout;
}

void SearchOptions::setCustomOptions(const QVariantMap& options)
{
    d->customOptions = options;
}

void SearchOptions::setSearchPaths(const QStringList& paths)
{
    d->searchPaths = paths;
}

void SearchOptions::setExcludePaths(const QStringList& paths)
{
    d->excludePaths = paths;
}

void SearchOptions::setFileFilters(const QStringList& filters)
{
    d->fileFilters = filters;
}

// SearchResult 实现
class SearchResult::Impl {
public:
    std::vector<ResultItemPtr> items;
    bool hasMoreResults{false};
    int totalResultCount{0};
    int currentPage{0};
    double searchTime{0.0};
};

SearchResult::SearchResult()
    : d(new Impl)
{
}

SearchResult::~SearchResult() = default;

const std::vector<SearchResult::ResultItemPtr>& SearchResult::items() const
{
    return d->items;
}

bool SearchResult::hasMoreResults() const
{
    return d->hasMoreResults;
}

int SearchResult::totalResultCount() const
{
    return d->totalResultCount;
}

int SearchResult::currentPage() const
{
    return d->currentPage;
}

double SearchResult::searchTime() const
{
    return d->searchTime;
}

void SearchResult::setHasMoreResults(bool hasMore)
{
    d->hasMoreResults = hasMore;
}

void SearchResult::setTotalResultCount(int count)
{
    d->totalResultCount = count;
}

void SearchResult::setCurrentPage(int page)
{
    d->currentPage = page;
}

void SearchResult::setSearchTime(double time)
{
    d->searchTime = time;
}

void SearchResult::addItem(ResultItemPtr item)
{
    d->items.push_back(std::move(item));
}

void SearchResult::clear()
{
    d->items.clear();
    d->hasMoreResults = false;
    d->totalResultCount = 0;
    d->currentPage = 0;
    d->searchTime = 0.0;
}

// SearchQuery 实现
class SearchQuery::Impl {
public:
    QString queryString;
    SearchType type{SearchType::Filename};
    SearchMechanism mechanism{SearchMechanism::Realtime};
    SearchOptions options;
};

SearchQuery::SearchQuery()
    : d(new Impl)
{
}

SearchQuery::~SearchQuery() = default;

QString SearchQuery::queryString() const
{
    return d->queryString;
}

SearchType SearchQuery::type() const
{
    return d->type;
}

SearchMechanism SearchQuery::mechanism() const
{
    return d->mechanism;
}

const SearchOptions& SearchQuery::options() const
{
    return d->options;
}

SearchOptions& SearchQuery::options()
{
    return d->options;
}

void SearchQuery::setQueryString(const QString& query)
{
    d->queryString = query;
}

void SearchQuery::setType(SearchType type)
{
    d->type = type;
}

void SearchQuery::setMechanism(SearchMechanism mechanism)
{
    d->mechanism = mechanism;
}

void SearchQuery::setOptions(const SearchOptions& options)
{
    d->options = options;
}

SearchQuery SearchQuery::createFilenameQuery(const QString& query)
{
    SearchQuery result;
    result.setQueryString(query);
    result.setType(SearchType::Filename);
    return result;
}

SearchQuery SearchQuery::createContentQuery(const QString& query)
{
    SearchQuery result;
    result.setQueryString(query);
    result.setType(SearchType::FileContent);
    return result;
}

SearchQuery SearchQuery::createAppQuery(const QString& query)
{
    SearchQuery result;
    result.setQueryString(query);
    result.setType(SearchType::Application);
    return result;
}

/**
 * @brief SearchEngine的私有实现类
 */
class SearchEngine::Impl {
public:
    Impl(SearchEngine* q)
        : q_ptr(q)
        , status(SearchStatus::Ready)
        , progressPercentage(0)
    {
    }
    
    // 引用外部类
    SearchEngine* q_ptr;
    
    // 状态相关
    SearchStatus status;
    int progressPercentage;
    SearchQuery currentQuery;
    SearchResult currentResults;
    
    // 互斥锁，保护状态和结果
    mutable QMutex mutex;
    
    // 回调函数
    ResultCallback resultCallback;
    ProgressCallback progressCallback;
    StatusCallback statusCallback;
    
    // 辅助方法
    void setStatus(SearchStatus newStatus) {
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
            if (status == SearchStatus::Completed) {
                emit q_ptr->searchCompleted();
            } else if (status == SearchStatus::Cancelled) {
                emit q_ptr->searchCancelled();
            } else if (status == SearchStatus::Error) {
                emit q_ptr->searchError("搜索过程中发生错误");
            }
        }
    }
    
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
    
    void addResults(const SearchResult& newResults) {
        QMutexLocker locker(&mutex);
        
        // 合并结果
        for (const auto& item : newResults.items()) {
            currentResults.addItem(item);
        }
        
        currentResults.setHasMoreResults(newResults.hasMoreResults());
        currentResults.setTotalResultCount(currentResults.items().size());
        currentResults.setSearchTime(newResults.searchTime());
        
        // 调用回调
        if (resultCallback) {
            resultCallback(currentResults);
        }
        
        // 发送信号
        emit q_ptr->resultsReady(currentResults);
    }
    
    void clearResults() {
        QMutexLocker locker(&mutex);
        currentResults.clear();
    }
};

// SearchEngine实现
SearchEngine::SearchEngine()
    : d(new Impl(this))
{
}

SearchEngine::~SearchEngine()
{
    // 确保搜索在析构前停止
    cancel();
}

bool SearchEngine::search(const SearchQuery& query)
{
    if (d->status == SearchStatus::Running || d->status == SearchStatus::Paused) {
        // 先取消当前搜索
        cancel();
    }
    
    // 保存查询
    d->currentQuery = query;
    
    // 清除之前的结果
    d->clearResults();
    
    // 更新状态
    d->setStatus(SearchStatus::Running);
    d->setProgress(0);
    
    // 这里应该启动实际的搜索实现
    // 由子类实现具体搜索逻辑
    
    return true;
}

bool SearchEngine::pause()
{
    if (d->status == SearchStatus::Running) {
        d->setStatus(SearchStatus::Paused);
        return true;
    }
    return false;
}

bool SearchEngine::resume()
{
    if (d->status == SearchStatus::Paused) {
        d->setStatus(SearchStatus::Running);
        return true;
    }
    return false;
}

bool SearchEngine::cancel()
{
    if (d->status == SearchStatus::Running || d->status == SearchStatus::Paused) {
        d->setStatus(SearchStatus::Cancelled);
        return true;
    }
    return false;
}

SearchStatus SearchEngine::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

int SearchEngine::progressPercentage() const
{
    QMutexLocker locker(&d->mutex);
    return d->progressPercentage;
}

SearchResult SearchEngine::getResults(int pageIndex, int pageSize) const
{
    QMutexLocker locker(&d->mutex);
    
    // 创建一个结果副本
    SearchResult result = d->currentResults;
    
    // 如果请求分页
    if (pageSize > 0) {
        // 计算分页
        int startIdx = pageIndex * pageSize;
        int endIdx = std::min(startIdx + pageSize, static_cast<int>(result.items().size()));
        
        if (startIdx < endIdx) {
            std::vector<SearchResult::ResultItemPtr> pagedItems(
                result.items().begin() + startIdx, 
                result.items().begin() + endIdx
            );
            
            for (const auto& item : pagedItems) {
                result.addItem(item);
            }
            result.setCurrentPage(pageIndex);
            result.setHasMoreResults(endIdx < static_cast<int>(d->currentResults.items().size()));
        } else {
            // 超出范围，返回空结果
            result.clear();
            result.setHasMoreResults(false);
        }
    }
    
    return result;
}

void SearchEngine::setResultCallback(ResultCallback callback)
{
    QMutexLocker locker(&d->mutex);
    d->resultCallback = std::move(callback);
}

void SearchEngine::setProgressCallback(ProgressCallback callback)
{
    QMutexLocker locker(&d->mutex);
    d->progressCallback = std::move(callback);
}

void SearchEngine::setStatusCallback(StatusCallback callback)
{
    QMutexLocker locker(&d->mutex);
    d->statusCallback = std::move(callback);
}

} // namespace Search
} // namespace DFM 