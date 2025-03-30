#pragma once

#include "global.h"
#include "searchquery.h"
#include "searchresult.h"
#include <QString>
#include <QObject>
#include <functional>
#include <memory>

namespace DFMSearch {

class SearchProviderPrivate;

/**
 * @brief 搜索提供者基类
 * 
 * 定义搜索提供者的通用接口，各类特定搜索实现需继承此类
 * 采用信号槽方式通知搜索结果和状态
 */
class DFMSEARCH_EXPORT SearchProvider : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SearchProvider(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SearchProvider() override;

    /**
     * @brief 获取提供者名称
     * @return 名称
     */
    virtual QString name() const = 0;
    
    /**
     * @brief 获取提供者描述
     * @return 描述
     */
    virtual QString description() const = 0;
    
    /**
     * @brief 获取支持的搜索类型
     * @return 搜索类型
     */
    virtual SearchType searchType() const = 0;
    
    /**
     * @brief 获取搜索机制
     * @return 搜索机制
     */
    virtual SearchMechanism mechanism() const = 0;
    
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
     * @brief 获取当前搜索状态
     * @return 搜索状态
     */
    SearchStatus status() const;
    
    /**
     * @brief 启动搜索
     * @return 是否成功启动
     */
    virtual bool start();
    
    /**
     * @brief 暂停搜索
     * @return 是否成功暂停
     */
    virtual bool pause();
    
    /**
     * @brief 恢复搜索
     * @return 是否成功恢复
     */
    virtual bool resume();
    
    /**
     * @brief 停止搜索
     * @return 是否成功停止
     */
    virtual bool stop();
    
    /**
     * @brief 设置结果回调函数
     * @param callback 回调函数
     */
    void setResultCallback(std::function<void(const SearchResult&)> callback);
    
    /**
     * @brief 设置搜索完成回调函数
     * @param callback 回调函数
     */
    void setCompletedCallback(std::function<void()> callback);
    
    /**
     * @brief 设置错误回调函数
     * @param callback 回调函数
     */
    void setErrorCallback(std::function<void(const QString&)> callback);

signals:
    /**
     * @brief 搜索状态变化信号
     * @param status 新状态
     */
    void statusChanged(DFMSearch::SearchStatus status);
    
    /**
     * @brief 搜索进度变化信号
     * @param progress 进度（0-100）
     */
    void progressChanged(int progress);
    
    /**
     * @brief 搜索结果信号
     * @param result 搜索结果
     */
    void resultFound(const DFMSearch::SearchResult& result);
    
    /**
     * @brief 搜索完成信号
     */
    void searchCompleted();
    
    /**
     * @brief 搜索错误信号
     * @param error 错误信息
     */
    void searchError(const QString& error);

protected:
    /**
     * @brief 设置搜索状态
     * @param status 状态
     */
    void setStatus(SearchStatus status);
    
    /**
     * @brief 添加搜索结果
     * @param result 结果
     */
    void addResult(const SearchResult& result);
    
    /**
     * @brief 设置进度
     * @param progress 进度（0-100）
     */
    void setProgress(int progress);
    
    /**
     * @brief 报告错误
     * @param error 错误信息
     */
    void reportError(const QString& error);
    
    /**
     * @brief 报告搜索完成
     */
    void reportCompleted();

    /**
     * @brief 执行实际搜索
     * 
     * 子类必须实现此方法执行具体搜索逻辑
     * @return 是否成功
     */
    virtual bool doSearch() = 0;

private:
    std::unique_ptr<SearchProviderPrivate> d;
};

// 工厂函数类型
using ProviderFactory = std::function<std::shared_ptr<SearchProvider>()>;

} // namespace DFMSearch 