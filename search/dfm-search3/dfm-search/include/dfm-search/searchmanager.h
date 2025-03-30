#pragma once

#include "global.h"
#include "searchquery.h"
#include "searchresult.h"
#include "searchprovider.h"
#include <QObject>
#include <memory>
#include <vector>
#include <functional>

namespace DFMSearch {

class SearchManagerPrivate;

/**
 * @brief 搜索管理器类
 * 
 * 管理所有搜索提供者，并提供统一的搜索接口
 */
class DFMSEARCH_EXPORT SearchManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取全局搜索管理器实例
     * @return 搜索管理器实例
     */
    static SearchManager* instance();
    
    /**
     * @brief 析构函数
     */
    ~SearchManager();
    
    /**
     * @brief 注册搜索提供者
     * @param factory 提供者工厂函数
     * @param searchType 搜索类型
     * @return 是否注册成功
     */
    bool registerProvider(ProviderFactory factory, SearchType searchType);
    
    /**
     * @brief 取消注册搜索提供者
     * @param searchType 搜索类型
     * @return 是否取消成功
     */
    bool unregisterProvider(SearchType searchType);
    
    /**
     * @brief 获取指定类型的搜索提供者
     * @param type 搜索类型
     * @return 搜索提供者指针
     */
    SearchProvider* provider(SearchType type);
    
    /**
     * @brief 获取所有搜索提供者
     * @return 搜索提供者列表
     */
    std::vector<SearchProvider*> providers() const;
    
    /**
     * @brief 设置搜索查询
     * @param query 搜索查询
     */
    void setQuery(const SearchQuery& query);
    
    /**
     * @brief 获取搜索查询
     * @return 搜索查询
     */
    SearchQuery query() const;
    
    /**
     * @brief 设置搜索类型
     * @param types 搜索类型集合
     */
    void setSearchTypes(const std::vector<SearchType>& types);
    
    /**
     * @brief 获取搜索类型
     * @return 搜索类型集合
     */
    std::vector<SearchType> searchTypes() const;
    
    /**
     * @brief 开始搜索
     * @return 是否成功开始
     */
    bool start();
    
    /**
     * @brief 暂停搜索
     * @return 是否成功暂停
     */
    bool pause();
    
    /**
     * @brief 恢复搜索
     * @return 是否成功恢复
     */
    bool resume();
    
    /**
     * @brief 停止搜索
     * @return 是否成功停止
     */
    bool stop();
    
    /**
     * @brief 是否正在搜索
     * @return 是否搜索中
     */
    bool isSearching() const;
    
    /**
     * @brief 获取搜索结果
     * @return 搜索结果列表
     */
    QList<SearchResult> results() const;
    
    /**
     * @brief 设置结果处理回调
     * @param callback 回调函数
     */
    void setResultHandler(std::function<void(const SearchResult&)> callback);
    
    /**
     * @brief 清空搜索结果
     */
    void clearResults();

signals:
    /**
     * @brief 搜索状态变化信号
     * @param status 新状态
     */
    void statusChanged(DFMSearch::SearchStatus status);
    
    /**
     * @brief 搜索开始信号
     */
    void searchStarted();
    
    /**
     * @brief 搜索暂停信号
     */
    void searchPaused();
    
    /**
     * @brief 搜索恢复信号
     */
    void searchResumed();
    
    /**
     * @brief 搜索停止信号
     */
    void searchStopped();
    
    /**
     * @brief 搜索完成信号
     */
    void searchCompleted();
    
    /**
     * @brief 搜索错误信号
     * @param error 错误信息
     */
    void searchError(const QString& error);
    
    /**
     * @brief 搜索结果信号
     * @param result 搜索结果
     */
    void resultFound(const DFMSearch::SearchResult& result);
    
    /**
     * @brief 搜索进度信号
     * @param progress 总体进度（0-100）
     */
    void progressChanged(int progress);

private:
    /**
     * @brief 构造函数（私有）
     * @param parent 父对象
     */
    explicit SearchManager(QObject* parent = nullptr);
    
    // 禁止复制
    SearchManager(const SearchManager&) = delete;
    SearchManager& operator=(const SearchManager&) = delete;

private:
    std::unique_ptr<SearchManagerPrivate> d;
};

} // namespace DFMSearch 