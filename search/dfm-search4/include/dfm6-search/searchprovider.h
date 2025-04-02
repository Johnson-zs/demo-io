#ifndef DFM6_SEARCH_PROVIDER_H
#define DFM6_SEARCH_PROVIDER_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchoptions.h>
#include <dfm6-search/searchresult.h>

#include <QObject>
#include <QString>
#include <memory>

namespace DFM6 {
namespace Search {

class SearchProviderData;

/**
 * @brief 搜索提供者接口类
 * 
 * 定义搜索提供者必须实现的接口，用于扩展搜索功能
 */
class DFM6_SEARCH_EXPORT SearchProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     */
    explicit SearchProvider(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SearchProvider() override;
    
    /**
     * @brief 获取提供者ID
     */
    virtual QString id() const = 0;
    
    /**
     * @brief 获取提供者名称
     */
    virtual QString name() const = 0;
    
    /**
     * @brief 获取提供者描述
     */
    virtual QString description() const = 0;
    
    /**
     * @brief 获取提供者版本
     */
    virtual QString version() const = 0;
    
    /**
     * @brief 检查提供者是否支持指定搜索类型
     */
    virtual bool supportsType(SearchType type) const = 0;
    
    /**
     * @brief 检查提供者是否支持指定搜索方法
     */
    virtual bool supportsMethod(SearchMethod method) const = 0;
    
    /**
     * @brief 开始搜索
     * 
     * @param query 搜索查询
     * @param options 搜索选项
     * @return 操作是否成功
     */
    virtual bool startSearch(const SearchQuery &query, const SearchOptions &options) = 0;
    
    /**
     * @brief 暂停搜索
     */
    virtual void pauseSearch() = 0;
    
    /**
     * @brief 恢复搜索
     */
    virtual void resumeSearch() = 0;
    
    /**
     * @brief 取消搜索
     */
    virtual void cancelSearch() = 0;
    
    /**
     * @brief 清除缓存
     */
    virtual void clearCache() = 0;
    
    /**
     * @brief 检查索引状态
     * 
     * @return 索引是否可用
     */
    virtual bool isIndexAvailable() const = 0;
    
    /**
     * @brief 获取当前状态
     */
    virtual SearchStatus status() const = 0;

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
     */
    void progressChanged(int current, int total);
    
    /**
     * @brief 搜索状态变更信号
     */
    void statusChanged(DFM6::Search::SearchStatus status);
    
    /**
     * @brief 搜索完成信号
     */
    void searchFinished(const QList<DFM6::Search::SearchResult> &results);
    
    /**
     * @brief 搜索取消信号
     */
    void searchCancelled();
    
    /**
     * @brief 搜索错误信号
     */
    void error(const QString &message);

private:
    std::unique_ptr<SearchProviderData> d;  // PIMPL
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_PROVIDER_H 