#include <dfm6-search/searchengine.h>
#include "../filenamesearch/filenamesearchengine.h"
#include "../contentsearch/contentsearchengine.h"
#include <QtConcurrent>

namespace DFM6 {
namespace Search {

SearchEngine::SearchEngine(QObject *parent)
    : QObject(parent),
      d_ptr(nullptr)
{
    // 默认创建文件名搜索引擎
    setSearchType(SearchType::FileName);
}

SearchEngine::SearchEngine(SearchType type, QObject *parent)
    : QObject(parent),
      d_ptr(nullptr)
{
    setSearchType(type);
}

SearchEngine::~SearchEngine() = default;

SearchType SearchEngine::searchType() const
{
    if (d_ptr) {
        return d_ptr->searchType();
    }
    return SearchType::FileName;  // 默认
}

void SearchEngine::setSearchType(SearchType type)
{
    // 如果类型相同且引擎已存在，则不重新创建
    if (d_ptr && d_ptr->searchType() == type) {
        return;
    }
    
    // 根据类型创建相应的引擎
    switch (type) {
    case SearchType::FileName:
        d_ptr = std::make_unique<FileNameSearchEngine>();
        break;
    case SearchType::Content:
        d_ptr = std::make_unique<ContentSearchEngine>();
        break;
    default:
        qWarning("Unsupported search type: %d", static_cast<int>(type));
        return;
    }
    
    // 连接信号
    connect(d_ptr.get(), &AbstractSearchEngine::searchStarted,
            this, &SearchEngine::searchStarted);
    connect(d_ptr.get(), &AbstractSearchEngine::resultFound,
            this, &SearchEngine::resultFound);
    connect(d_ptr.get(), &AbstractSearchEngine::progressChanged,
            this, &SearchEngine::progressChanged);
    connect(d_ptr.get(), &AbstractSearchEngine::statusChanged,
            this, &SearchEngine::statusChanged);
    connect(d_ptr.get(), &AbstractSearchEngine::searchFinished,
            this, &SearchEngine::searchFinished);
    connect(d_ptr.get(), &AbstractSearchEngine::searchCancelled,
            this, &SearchEngine::searchCancelled);
    connect(d_ptr.get(), &AbstractSearchEngine::error,
            this, &SearchEngine::error);
}

SearchOptions SearchEngine::searchOptions() const
{
    if (d_ptr) {
        return d_ptr->searchOptions();
    }
    return SearchOptions();
}

void SearchEngine::setSearchOptions(const SearchOptions &options)
{
    if (d_ptr) {
        d_ptr->setSearchOptions(options);
    }
}

SearchStatus SearchEngine::status() const
{
    if (d_ptr) {
        return d_ptr->status();
    }
    return SearchStatus::Error;
}

QFuture<QList<SearchResult>> SearchEngine::search(const SearchQuery &query)
{
    if (d_ptr) {
        return d_ptr->search(query);
    }
    return QtConcurrent::run([]() { return QList<SearchResult>(); });
}

QFuture<void> SearchEngine::searchWithCallback(const SearchQuery &query, ResultCallback callback)
{
    if (d_ptr) {
        return d_ptr->searchWithCallback(query, callback);
    }
    return QtConcurrent::run([]() {});
}

QList<SearchResult> SearchEngine::searchSync(const SearchQuery &query)
{
    if (d_ptr) {
        return d_ptr->searchSync(query);
    }
    return QList<SearchResult>();
}

void SearchEngine::pause()
{
    if (d_ptr) {
        d_ptr->pause();
    }
}

void SearchEngine::resume()
{
    if (d_ptr) {
        d_ptr->resume();
    }
}

void SearchEngine::cancel()
{
    if (d_ptr) {
        d_ptr->cancel();
    }
}

void SearchEngine::clearCache()
{
    if (d_ptr) {
        d_ptr->clearCache();
    }
}

FileNameSearchAPI SearchEngine::fileNameOptions()
{
    SearchOptions& options = searchOptions();
    return FileNameSearchAPI(options);
}

ContentSearchAPI SearchEngine::contentOptions()
{
    SearchOptions& options = searchOptions();
    return ContentSearchAPI(options);
}

SearchEngine* SearchEngine::create(SearchType type, QObject *parent)
{
    // 这是客户端唯一可用的创建引擎的方法
    return SearchFactory::createEngine(type, parent);
}

void SearchEngine::registerProvider(const QString &name, std::shared_ptr<SearchProvider> provider)
{
    SearchFactory::instance().registerProvider(provider);
}

}  // namespace Search
}  // namespace DFM6 