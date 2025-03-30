#include "dfm-search/searchprovider.h"

namespace DFMSearch {

// 私有实现类
class SearchProviderPrivate {
public:
    SearchQuery query;
    SearchStatus status = SearchStatus::Ready;
    int progress = 0;
    std::function<void(const SearchResult&)> resultCallback;
    std::function<void()> completedCallback;
    std::function<void(const QString&)> errorCallback;
};

SearchProvider::SearchProvider(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<SearchProviderPrivate>())
{
}

SearchProvider::~SearchProvider() = default;

void SearchProvider::setQuery(const SearchQuery& query)
{
    d->query = query;
}

SearchQuery SearchProvider::query() const
{
    return d->query;
}

SearchStatus SearchProvider::status() const
{
    return d->status;
}

bool SearchProvider::start()
{
    if (d->status == SearchStatus::Searching || d->status == SearchStatus::Paused) {
        return false;
    }
    
    setStatus(SearchStatus::Searching);
    setProgress(0);
    
    return doSearch();
}

bool SearchProvider::pause()
{
    if (d->status != SearchStatus::Searching) {
        return false;
    }
    
    setStatus(SearchStatus::Paused);
    return true;
}

bool SearchProvider::resume()
{
    if (d->status != SearchStatus::Paused) {
        return false;
    }
    
    setStatus(SearchStatus::Searching);
    return doSearch();
}

bool SearchProvider::stop()
{
    if (d->status != SearchStatus::Searching && d->status != SearchStatus::Paused) {
        return false;
    }
    
    setStatus(SearchStatus::Ready);
    return true;
}

void SearchProvider::setResultCallback(std::function<void(const SearchResult&)> callback)
{
    d->resultCallback = callback;
}

void SearchProvider::setCompletedCallback(std::function<void()> callback)
{
    d->completedCallback = callback;
}

void SearchProvider::setErrorCallback(std::function<void(const QString&)> callback)
{
    d->errorCallback = callback;
}

void SearchProvider::setStatus(SearchStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged(status);
    }
}

void SearchProvider::addResult(const SearchResult& result)
{
    emit resultFound(result);
    
    if (d->resultCallback) {
        d->resultCallback(result);
    }
}

void SearchProvider::setProgress(int progress)
{
    if (d->progress != progress) {
        d->progress = progress;
        emit progressChanged(progress);
    }
}

void SearchProvider::reportError(const QString& error)
{
    emit searchError(error);
    
    if (d->errorCallback) {
        d->errorCallback(error);
    }
    
    setStatus(SearchStatus::Error);
}

void SearchProvider::reportCompleted()
{
    emit searchCompleted();
    
    if (d->completedCallback) {
        d->completedCallback();
    }
    
    setStatus(SearchStatus::Completed);
}

} // namespace DFMSearch 