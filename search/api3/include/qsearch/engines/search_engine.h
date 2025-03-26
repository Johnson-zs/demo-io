#pragma once

#include <QObject>
#include <QSharedPointer>
#include "../search_query.h"
#include "../search_result.h"
#include "../search_options.h"
#include "../global.h"

namespace QSearch {

class QSEARCH_EXPORT SearchEngine : public QObject {
    Q_OBJECT
public:
    explicit SearchEngine(QObject* parent = nullptr);
    virtual ~SearchEngine();
    
    // 引擎标识和信息
    virtual QString engineId() const = 0;
    virtual QString engineName() const = 0;
    virtual QString engineDescription() const = 0;
    
    // 能力查询
    virtual bool supportsIndexing() const = 0;
    virtual bool supportsQueryType(QueryType type) const = 0;
    virtual bool supportsMatchType(MatchType type) const = 0;
    virtual bool supportsFeature(const QString& feature) const = 0;
    
    // 搜索操作
    virtual bool prepare(const SearchQuery& query, const SearchOptions& options) = 0;
    virtual bool start() = 0;
    virtual bool pause() = 0;
    virtual bool resume() = 0;
    virtual bool stop() = 0;
    
    // 获取当前状态
    virtual SearchState state() const = 0;
    virtual double progress() const = 0;
    virtual SearchResult currentResults() const = 0;
    
signals:
    void resultItemFound(const ResultItem& item);
    void progressChanged(double progress);
    void stateChanged(SearchState state);
    void error(const QString& errorMessage);
    void searchCompleted();
};

} // namespace QSearch 