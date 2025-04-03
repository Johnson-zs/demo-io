#ifndef CONTENT_SEARCH_ENGINE_H
#define CONTENT_SEARCH_ENGINE_H

#include "../core/abstractsearchengine.h"
#include "contentsearchoptions.h"
#include <memory>

// 前向声明
class ContentSearcher;

namespace DFM6 {
namespace Search {

/**
 * @brief 内容搜索引擎
 *
 * 实现基于文件内容的搜索功能
 */
class ContentSearchEngine : public AbstractSearchEngine
{
    Q_OBJECT

public:
    explicit ContentSearchEngine(QObject *parent = nullptr);
    ~ContentSearchEngine() override;

    // 实现AbstractSearchEngine接口
    SearchType searchType() const override { return SearchType::Content; }
    void setSearchType(SearchType) override { }   // 类型固定为Content

    SearchOptions searchOptions() const override;
    void setSearchOptions(const SearchOptions &options) override;

    SearchStatus status() const override;

    QFuture<QList<SearchResult>> search(const SearchQuery &query) override;
    QFuture<void> searchWithCallback(const SearchQuery &query,
                                     SearchEngine::ResultCallback callback) override;
    QList<SearchResult> searchSync(const SearchQuery &query) override;

    void pause() override;
    void resume() override;
    void cancel() override;
    void clearCache() override;

private:
    // 将SearchQuery转换为底层搜索引擎能理解的格式
    QStringList processQuery(const SearchQuery &query);

    // 转换搜索结果为标准格式
    QList<SearchResult> convertResults(const QList<SearchResult> &results);

    std::unique_ptr<ContentSearcher> m_searcher;
    ContentSearchOptions m_options;
    SearchQuery m_currentQuery;   // 当前查询
};

}   // namespace Search
}   // namespace DFM6

#endif   // CONTENT_SEARCH_ENGINE_H
