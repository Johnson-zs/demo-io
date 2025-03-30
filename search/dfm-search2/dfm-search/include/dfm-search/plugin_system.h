#ifndef DFM_PLUGIN_SYSTEM_H
#define DFM_PLUGIN_SYSTEM_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QPluginLoader>

#include "search_engine.h" // 引入SearchResult等相关类型

namespace DFM {
namespace Search {

/**
 * @brief 插件接口基类
 * 
 * 所有搜索插件必须实现此接口
 */
class SearchPlugin {
public:
    virtual ~SearchPlugin() = default;
    
    /**
     * @brief 获取插件标识符
     * @return 插件唯一标识符
     */
    virtual QString pluginId() const = 0;
    
    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    virtual QString name() const = 0;
    
    /**
     * @brief 获取插件描述
     * @return 插件描述
     */
    virtual QString description() const = 0;
    
    /**
     * @brief 获取插件版本
     * @return 插件版本
     */
    virtual QString version() const = 0;
    
    /**
     * @brief 获取插件作者
     * @return 插件作者
     */
    virtual QString author() const = 0;
    
    /**
     * @brief 初始化插件
     * @param conf 插件配置参数
     * @return 是否初始化成功
     */
    virtual bool initialize(const QVariantMap& conf = {}) = 0;
    
    /**
     * @brief 关闭插件
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 获取插件支持的搜索类型
     * @return 支持的搜索类型列表
     */
    virtual QList<SearchType> supportedSearchTypes() const = 0;
    
    /**
     * @brief 获取插件支持的搜索机制
     * @return 支持的搜索机制列表
     */
    virtual QList<SearchMechanism> supportedSearchMechanisms() const = 0;
    
    /**
     * @brief 创建搜索引擎实例
     * @param type 搜索类型
     * @param mechanism 搜索机制
     * @return 搜索引擎实例
     */
    virtual std::shared_ptr<SearchEngine> createSearchEngine(
        SearchType type, SearchMechanism mechanism) = 0;
};

/**
 * @brief 搜索插件接口（Qt 插件接口）
 * 
 * 用于Qt插件系统加载插件
 */
class SearchPluginInterface {
public:
    virtual ~SearchPluginInterface() = default;
    virtual std::shared_ptr<SearchPlugin> createPlugin() = 0;
};

#define SearchPluginInterface_iid "com.deepin.dfm.SearchPluginInterface/1.0"
Q_DECLARE_INTERFACE(DFM::Search::SearchPluginInterface, SearchPluginInterface_iid)

/**
 * @brief 插件管理器类
 * 
 * 负责加载、管理和使用搜索插件
 */
class PluginManager : public QObject {
    Q_OBJECT
    
public:
    static PluginManager& instance();
    
    // 插件加载和管理
    bool loadPlugin(const QString& pluginPath);
    bool loadPluginsFromDirectory(const QString& directory);
    bool unloadPlugin(const QString& pluginId);
    void unloadAllPlugins();
    
    // 插件查询
    std::vector<std::shared_ptr<SearchPlugin>> getPlugins() const;
    std::shared_ptr<SearchPlugin> getPlugin(const QString& pluginId) const;
    std::vector<std::shared_ptr<SearchPlugin>> getPluginsForSearchType(SearchType type) const;
    
    // 引擎创建
    std::shared_ptr<SearchEngine> createSearchEngine(
        const QString& pluginId, SearchType type, SearchMechanism mechanism);
    
    // 引擎推荐
    std::shared_ptr<SearchPlugin> suggestPluginForSearch(
        SearchType type, SearchMechanism mechanism) const;
    
signals:
    void pluginLoaded(const QString& pluginId);
    void pluginUnloaded(const QString& pluginId);
    void pluginError(const QString& pluginId, const QString& errorMessage);
    
private:
    PluginManager();
    ~PluginManager();
    
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 注册C++内置插件
 * 
 * 用于注册非动态加载的内置插件
 */
class BuiltinPluginRegistry {
public:
    static BuiltinPluginRegistry& instance();
    
    // 注册内置插件
    void registerPlugin(std::shared_ptr<SearchPlugin> plugin);
    
    // 获取所有内置插件
    std::vector<std::shared_ptr<SearchPlugin>> getPlugins() const;
    
private:
    BuiltinPluginRegistry() = default;
    ~BuiltinPluginRegistry() = default;
    
    BuiltinPluginRegistry(const BuiltinPluginRegistry&) = delete;
    BuiltinPluginRegistry& operator=(const BuiltinPluginRegistry&) = delete;
    
    std::vector<std::shared_ptr<SearchPlugin>> plugins_;
};

} // namespace Search
} // namespace DFM

#endif // DFM_PLUGIN_SYSTEM_H 