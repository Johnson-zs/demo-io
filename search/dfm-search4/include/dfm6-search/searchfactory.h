#ifndef DFM6_SEARCH_FACTORY_H
#define DFM6_SEARCH_FACTORY_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchprovider.h>
#include <dfm6-search/searchoptions.h>
#include <dfm6-search/searchresult.h>

#include <memory>

namespace DFM6 {
namespace Search {

class SearchFactoryData;

/**
 * @brief 搜索工厂类
 *
 * 创建和管理搜索引擎实例和相关对象
 */
class DFM6_SEARCH_EXPORT SearchFactory
{
public:
    /**
     * @brief 获取单例实例
     */
    static SearchFactory &instance();

    /**
     * @brief 创建搜索引擎
     *
     * 这是创建SearchEngine的内部实现方法，
     * 客户端代码应使用SearchEngine::create()
     */
    static SearchEngine *createEngine(SearchType type, QObject *parent = nullptr);

    /**
     * @brief 创建搜索查询
     */
    static SearchQuery createQuery(const QString &keyword, SearchQuery::Type type = SearchQuery::Type::Simple);

    /**
     * @brief 注册自定义搜索引擎类型
     */
    static void registerEngineCreator(SearchType type, std::function<std::shared_ptr<AbstractSearchEngine>(QObject *)> creator);

    /**
     * @brief 注册搜索提供者
     *
     * @param provider 搜索提供者
     * @return 注册是否成功
     */
    bool registerProvider(std::shared_ptr<SearchProvider> provider);

    /**
     * @brief 注销搜索提供者
     *
     * @param providerId 提供者ID
     * @return 注销是否成功
     */
    bool unregisterProvider(const QString &providerId);

    /**
     * @brief 获取已注册的提供者
     *
     * @return 提供者列表
     */
    QList<std::shared_ptr<SearchProvider>> providers() const;

    /**
     * @brief 根据ID获取提供者
     *
     * @param providerId 提供者ID
     * @return 提供者对象，如果不存在则返回nullptr
     */
    std::shared_ptr<SearchProvider> provider(const QString &providerId) const;

    /**
     * @brief 检查是否存在支持指定搜索类型的提供者
     *
     * @param type 搜索类型
     * @return 是否存在支持的提供者
     */
    bool hasProviderForType(SearchType type) const;

    /**
     * @brief 创建搜索选项
     *
     * @param type 搜索类型
     * @return 对应类型的搜索选项
     */
    std::unique_ptr<SearchOptions> createOptions(SearchType type);

    /**
     * @brief 创建搜索结果
     *
     * @param type 搜索类型
     * @param path 文件路径
     * @return 对应类型的搜索结果
     */
    std::unique_ptr<SearchResult> createResult(SearchType type, const QString &path);

private:
    /**
     * @brief 私有构造函数
     */
    SearchFactory();

    /**
     * @brief 私有析构函数
     */
    ~SearchFactory();

    // 禁止拷贝
    SearchFactory(const SearchFactory &) = delete;
    SearchFactory &operator=(const SearchFactory &) = delete;

    std::unique_ptr<SearchFactoryData> d;   // PIMPL
};

}   // namespace Search
}   // namespace DFM6

#endif   // DFM6_SEARCH_FACTORY_H
