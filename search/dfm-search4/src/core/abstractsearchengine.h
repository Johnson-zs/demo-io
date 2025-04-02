#ifndef ABSTRACT_SEARCH_ENGINE_H
#define ABSTRACT_SEARCH_ENGINE_H

#include <dfm6-search/searchengine.h>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <atomic>

namespace DFM6 {
namespace Search {

/**
 * @brief 抽象搜索引擎类
 * 
 * SearchEngine的私有实现基类
 */
class AbstractSearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSearchEngine(QObject *parent = nullptr);
    virtual ~AbstractSearchEngine();
    
    // 公共接口方法
    virtual SearchType searchType() const = 0;
    virtual void setSearchType(SearchType type) = 0;
    
    virtual SearchOptions searchOptions() const = 0;
    virtual void setSearchOptions(const SearchOptions &options) = 0;
    
    virtual SearchStatus status() const = 0;
    
    virtual QFuture<QList<SearchResult>> search(const SearchQuery &query) = 0;
    virtual QFuture<void> searchWithCallback(const SearchQuery &query, 
                                            SearchEngine::ResultCallback callback) = 0;
    virtual QList<SearchResult> searchSync(const SearchQuery &query) = 0;
    
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void cancel() = 0;
    virtual void clearCache() = 0;

signals:
    void searchStarted();
    void resultFound(const SearchResult &result);
    void progressChanged(int current, int total);
    void statusChanged(SearchStatus status);
    void searchFinished(const QList<SearchResult> &results);
    void searchCancelled();
    void error(const QString &message);

protected:
    // 辅助方法
    void setStatus(SearchStatus status);
    void reportProgress(int current, int total);
    void reportError(const QString &message);
    
    std::atomic<SearchStatus> m_status;
    std::atomic<bool> m_cancelled;
    QMutex m_mutex;
    QWaitCondition m_pauseCondition;
};

}  // namespace Search
}  // namespace DFM6

#endif // ABSTRACT_SEARCH_ENGINE_H 