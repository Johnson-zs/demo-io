#include <dfm-search/search_engine.h>
#include <dfm-search/dfm_search.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QPair>
#include <QString>

namespace DFM {
namespace Search {

// 内部使用的工厂类型
using EngineFactory = std::function<std::shared_ptr<SearchEngine>()>;

// 搜索引擎管理器的实现
class SearchEngineManager::Impl {
public:
    // 引擎工厂映射表
    // 键为类型和机制的组合
    using FactoryKey = QPair<SearchType, SearchMechanism>;
    QMap<FactoryKey, SearchEngineManager::EngineCreator> engineFactories;
    
    // 保护映射表的互斥锁
    QMutex mutex;
    
    // 查找工厂方法
    EngineCreator findFactory(SearchType type, SearchMechanism mechanism) const {
        QMutexLocker locker(&mutex);
        
        // 精确匹配
        FactoryKey key(type, mechanism);
        auto it = engineFactories.find(key);
        if (it != engineFactories.end()) {
            return it.value();
        }
        
        // 查找自定义类型的备选工厂
        if (type == SearchType::Custom) {
            // 尝试找到任何自定义搜索类型的工厂
            for (auto it = engineFactories.begin(); it != engineFactories.end(); ++it) {
                if (it.key().first == SearchType::Custom && it.key().second == mechanism) {
                    return it.value();
                }
            }
        }
        
        // 找不到匹配的工厂
        return nullptr;
    }
};

// 全局单例实例
SearchEngineManager& SearchEngineManager::instance() {
    static SearchEngineManager instance;
    return instance;
}

// 创建搜索引擎实例
std::shared_ptr<SearchEngine> SearchEngineManager::createEngine(
    SearchType type, SearchMechanism mechanism) {
    
    auto factory = d->findFactory(type, mechanism);
    if (factory) {
        return factory();
    }
    
    // 如果找不到工厂，尝试使用内建的插件
    auto plugin = suggestPluginForSearch(type, mechanism);
    if (plugin) {
        return plugin->createSearchEngine(type, mechanism);
    }
    
    qWarning() << "找不到搜索引擎工厂:" << static_cast<int>(type) << static_cast<int>(mechanism);
    return nullptr;
}

// 注册搜索引擎工厂
void SearchEngineManager::registerEngineCreator(
    SearchType type, SearchMechanism mechanism, EngineCreator creator) {
    
    QMutexLocker locker(&d->mutex);
    
    if (!creator) {
        qWarning() << "尝试注册空的引擎工厂";
        return;
    }
    
    Impl::FactoryKey key(type, mechanism);
    d->engineFactories[key] = std::move(creator);
}

// 构造函数
SearchEngineManager::SearchEngineManager()
    : d(new Impl())
{
}

// 析构函数
SearchEngineManager::~SearchEngineManager() {
}

// 便捷函数：创建文件名搜索引擎
std::shared_ptr<SearchEngine> createFilenameSearchEngine(bool indexed) {
    return SearchEngineManager::instance().createEngine(
        SearchType::Filename, 
        indexed ? SearchMechanism::Indexed : SearchMechanism::Realtime
    );
}

// 便捷函数：创建内容搜索引擎
std::shared_ptr<SearchEngine> createContentSearchEngine(bool indexed) {
    return SearchEngineManager::instance().createEngine(
        SearchType::FileContent, 
        indexed ? SearchMechanism::Indexed : SearchMechanism::Realtime
    );
}

// 获取库版本号
QString version() {
    return QStringLiteral("0.1.0");
}

// 初始化搜索库
bool initialize(const QStringList& pluginDirs) {
    // 这里应该初始化各种组件，加载插件等
    
    // 模拟初始化成功
    return true;
}

// 关闭搜索库
void shutdown() {
    // 清理资源
}

// 是否已初始化
bool isInitialized() {
    // 检查初始化状态
    return true;
}

} // namespace Search
} // namespace DFM 