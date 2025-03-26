#pragma once

#include "search_engine.h"
#include "../global.h"

namespace QSearch {

class QSEARCH_EXPORT FulltextSearchEngine : public SearchEngine {
    Q_OBJECT
public:
    explicit FulltextSearchEngine(QObject* parent = nullptr);
    ~FulltextSearchEngine() override;
    
    // 实现基类接口
    QString engineId() const override;
    QString engineName() const override;
    QString engineDescription() const override;
    
    bool supportsIndexing() const override;
    bool supportsQueryType(QueryType type) const override;
    bool supportsMatchType(MatchType type) const override;
    bool supportsFeature(const QString& feature) const override;
    
    bool prepare(const SearchQuery& query, const SearchOptions& options) override;
    bool start() override;
    bool pause() override;
    bool resume() override;
    bool stop() override;
    
    SearchState state() const override;
    double progress() const override;
    SearchResult currentResults() const override;
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 