#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFuture>
#include <functional>
#include "search_query.h"
#include "search_result.h"
#include "search_options.h"
#include "global.h"

namespace QSearch {

enum class SearchState {
    Idle,           // 空闲状态
    Searching,      // 搜索中
    Paused,         // 已暂停
    Completed,      // 已完成
    Error           // 错误状态
};

class QSEARCH_EXPORT SearchManager : public QObject {
    Q_OBJECT
public:
    static SearchManager& instance();
    
    // 搜索控制
    int startSearch(const SearchQuery& query, const SearchOptions& options = {});
    bool pauseSearch(int searchId);
    bool resumeSearch(int searchId);
    bool stopSearch(int searchId);
    
    // 搜索状态
    SearchState searchState(int searchId) const;
    QString searchStateMessage(int searchId) const;
    double searchProgress(int searchId) const;
    
    // 获取结果
    SearchResult getResults(int searchId) const;
    
    // 索引管理
    IndexManager* indexManager() const;
    
    // 注册回调处理
    void setResultCallback(int searchId, std::function<void(const ResultItem&)> callback);
    
    // 异步搜索
    QFuture<SearchResult> searchAsync(const SearchQuery& query, const SearchOptions& options = {});
    
    // 批量搜索
    SearchResult search(const SearchQuery& query, const SearchOptions& options = {});
    
signals:
    void searchStarted(int searchId);
    void searchPaused(int searchId);
    void searchResumed(int searchId);
    void searchStopped(int searchId);
    void searchCompleted(int searchId);
    void searchError(int searchId, const QString& errorMessage);
    void searchProgressChanged(int searchId, double progress);
    void resultItemFound(int searchId, const ResultItem& item);
    void resultsUpdated(int searchId);
    
private:
    // 私有构造函数(单例模式)
    SearchManager();
    ~SearchManager();
    
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 