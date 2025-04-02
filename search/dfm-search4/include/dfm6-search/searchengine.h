#ifndef DFM6_SEARCH_ENGINE_H
#define DFM6_SEARCH_ENGINE_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchoptions.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchresult.h>

#include <QObject>
#include <QString>
#include <QList>
#include <QFuture>
#include <functional>
#include <memory>

namespace DFM6 {
namespace Search {

class AbstractSearchEngine;

/**
 * @brief 搜索引擎类
 * 
 * 提供统一的搜索接口，内部使用具体的搜索引擎实现
 */
class DFM6_SEARCH_EXPORT SearchEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 搜索结果回调函数类型
     */
    using ResultCallback = std::function<void(const SearchResult&)>;
    
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象
     */
    explicit SearchEngine(QObject *parent = nullptr);
    
    /**
     * @brief 使用指定搜索类型构造
     * 
     * @param type 搜索类型
     * @param parent 父对象
     */
    SearchEngine(SearchType type, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SearchEngine() override;
    
    /**
     * @brief 获取搜索类型
     */
    SearchType searchType() const;
    
    /**
     * @brief 设置搜索类型
     */
    void setSearchType(SearchType type);
    
    /**
     * @brief 获取搜索选项
     */
    SearchOptions searchOptions() const;
    
    /**
     * @brief 设置搜索选项
     */
    void setSearchOptions(const SearchOptions &options);
    
    /**
     * @brief 获取当前搜索状态
     */
    SearchStatus status() const;
    
    /**
     * @brief 异步执行搜索
     * 
     * @param query 搜索查询
     * @return 代表异步操作的Future对象
     */
    QFuture<QList<SearchResult>> search(const SearchQuery &query);
    
    /**
     * @brief 异步执行搜索，并通过回调返回结果
     * 
     * @param query 搜索查询
     * @param callback 结果回调函数
     * @return 代表异步操作的Future对象
     */
    QFuture<void> searchWithCallback(const SearchQuery &query, ResultCallback callback);
    
    /**
     * @brief 同步执行搜索
     * 
     * @param query 搜索查询
     * @return 搜索结果列表
     */
    QList<SearchResult> searchSync(const SearchQuery &query);
    
    /**
     * @brief 暂停当前搜索
     */
    void pause();
    
    /**
     * @brief 恢复暂停的搜索
     */
    void resume();
    
    /**
     * @brief 取消当前搜索
     */
    void cancel();
    
    /**
     * @brief 清除缓存
     */
    void clearCache();

signals:
    /**
     * @brief 搜索开始信号
     */
    void searchStarted();
    
    /**
     * @brief 搜索结果信号
     */
    void resultFound(const DFM6::Search::SearchResult &result);
    
    /**
     * @brief 搜索进度信号
     * 
     * @param current 当前进度
     * @param total 总进度
     */
    void progressChanged(int current, int total);
    
    /**
     * @brief 搜索状态改变信号
     */
    void statusChanged(DFM6::Search::SearchStatus status);
    
    /**
     * @brief 搜索完成信号
     * 
     * @param results 搜索结果列表
     */
    void searchFinished(const QList<DFM6::Search::SearchResult> &results);
    
    /**
     * @brief 搜索取消信号
     */
    void searchCancelled();
    
    /**
     * @brief 搜索错误信号
     * 
     * @param message 错误消息
     */
    void error(const QString &message);

private:
    std::unique_ptr<AbstractSearchEngine> d_ptr;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_ENGINE_H 