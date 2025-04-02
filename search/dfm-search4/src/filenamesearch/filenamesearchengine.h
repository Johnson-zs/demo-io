#ifndef FILENAME_SEARCH_ENGINE_H
#define FILENAME_SEARCH_ENGINE_H

#include "../core/abstractsearchengine.h"
#include "filenamesearchoptions.h"
#include <memory>

// 前向声明
class LuceneSearchEngine;

namespace DFM6 {
namespace Search {

/**
 * @brief 文件名搜索引擎
 * 
 * 实现基于文件名的搜索功能
 */
class FileNameSearchEngine : public AbstractSearchEngine
{
    Q_OBJECT

public:
    explicit FileNameSearchEngine(QObject *parent = nullptr);
    ~FileNameSearchEngine() override;
    
    // 实现AbstractSearchEngine接口
    SearchType searchType() const override { return SearchType::FileName; }
    void setSearchType(SearchType) override {} // 类型固定为FileName
    
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
    // 将SearchQuery转换为适合底层引擎的格式
    QString convertQuery(const SearchQuery &query);
    
    // 将底层搜索引擎的结果转换为SearchResult
    QList<SearchResult> convertResults(const QVector<FileData> &fileDataList);
    
    std::unique_ptr<LuceneSearchEngine> m_engine;
    FileNameSearchOptions m_options;
};

}  // namespace Search
}  // namespace DFM6

#endif // FILENAME_SEARCH_ENGINE_H 