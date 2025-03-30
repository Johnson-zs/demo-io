#include "dfm-search/searchmanager.h"
#include <QMap>

namespace DFMSearch {

// 搜索管理器私有实现类
class SearchManagerPrivate {
public:
    QMap<SearchType, std::shared_ptr<SearchProvider>> providers;
    QMap<SearchType, ProviderFactory> providerFactories;
    std::vector<SearchType> activeTypes;
    SearchQuery query;
    QList<SearchResult> searchResults;
    std::function<void(const SearchResult&)> resultHandler;
};

// 静态实例
static SearchManager* s_instance = nullptr;

SearchManager* SearchManager::instance()
{
    if (!s_instance) {
        s_instance = new SearchManager;
    }
    return s_instance;
}

SearchManager::SearchManager(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<SearchManagerPrivate>())
{
}

SearchManager::~SearchManager()
{
    d->providers.clear();
    
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

bool SearchManager::registerProvider(ProviderFactory factory, SearchType searchType)
{
    if (d->providerFactories.contains(searchType)) {
        return false; // 已经注册了此类型
    }
    
    d->providerFactories[searchType] = factory;
    return true;
}

bool SearchManager::unregisterProvider(SearchType searchType)
{
    if (!d->providerFactories.contains(searchType)) {
        return false; // 未注册此类型
    }
    
    d->providerFactories.remove(searchType);
    d->providers.remove(searchType);
    
    // 如果在激活类型中，也一并移除
    auto it = std::find(d->activeTypes.begin(), d->activeTypes.end(), searchType);
    if (it != d->activeTypes.end()) {
        d->activeTypes.erase(it);
    }
    
    return true;
}

SearchProvider* SearchManager::provider(SearchType type)
{
    if (!d->providers.contains(type)) {
        if (d->providerFactories.contains(type)) {
            // 懒加载提供者
            auto factory = d->providerFactories[type];
            auto provider = factory();
            
            if (provider) {
                // 连接信号
                connect(provider.get(), &SearchProvider::statusChanged, 
                        this, &SearchManager::statusChanged);
                connect(provider.get(), &SearchProvider::progressChanged, 
                        this, &SearchManager::progressChanged);
                connect(provider.get(), &SearchProvider::resultFound, 
                        this, &SearchManager::resultFound);
                connect(provider.get(), &SearchProvider::searchError, 
                        this, &SearchManager::searchError);
                connect(provider.get(), &SearchProvider::searchCompleted, 
                        this, &SearchManager::searchCompleted);
                
                // 注册结果处理回调
                provider->setResultCallback([this](const SearchResult& result) {
                    d->searchResults.append(result);
                    if (d->resultHandler) {
                        d->resultHandler(result);
                    }
                });
                
                d->providers[type] = provider;
            }
        } else {
            return nullptr; // 未注册此类型
        }
    }
    
    return d->providers[type].get();
}

std::vector<SearchProvider*> SearchManager::providers() const
{
    std::vector<SearchProvider*> result;
    for (auto it = d->providers.begin(); it != d->providers.end(); ++it) {
        result.push_back(it.value().get());
    }
    return result;
}

void SearchManager::setQuery(const SearchQuery& query)
{
    d->query = query;
    
    // 同步到所有提供者
    for (auto it = d->providers.begin(); it != d->providers.end(); ++it) {
        it.value()->setQuery(query);
    }
}

SearchQuery SearchManager::query() const
{
    return d->query;
}

void SearchManager::setSearchTypes(const std::vector<SearchType>& types)
{
    d->activeTypes = types;
}

std::vector<SearchType> SearchManager::searchTypes() const
{
    return d->activeTypes;
}

bool SearchManager::start()
{
    if (isSearching()) {
        return false;
    }
    
    d->searchResults.clear();
    
    bool anyStarted = false;
    
    // 启动所有激活的类型
    for (auto type : d->activeTypes) {
        auto prov = provider(type);
        if (prov) {
            prov->setQuery(d->query);
            if (prov->start()) {
                anyStarted = true;
            }
        }
    }
    
    if (anyStarted) {
        emit searchStarted();
    }
    
    return anyStarted;
}

bool SearchManager::pause()
{
    if (!isSearching()) {
        return false;
    }
    
    bool anyPaused = false;
    
    for (auto type : d->activeTypes) {
        auto prov = provider(type);
        if (prov && prov->pause()) {
            anyPaused = true;
        }
    }
    
    if (anyPaused) {
        emit searchPaused();
    }
    
    return anyPaused;
}

bool SearchManager::resume()
{
    bool anyResumed = false;
    
    for (auto type : d->activeTypes) {
        auto prov = provider(type);
        if (prov && prov->resume()) {
            anyResumed = true;
        }
    }
    
    if (anyResumed) {
        emit searchResumed();
    }
    
    return anyResumed;
}

bool SearchManager::stop()
{
    bool anyStopped = false;
    
    for (auto type : d->activeTypes) {
        auto prov = provider(type);
        if (prov && prov->stop()) {
            anyStopped = true;
        }
    }
    
    if (anyStopped) {
        emit searchStopped();
    }
    
    return anyStopped;
}

bool SearchManager::isSearching() const
{
    for (auto type : d->activeTypes) {
        const auto it = d->providers.find(type);
        if (it != d->providers.end() && 
            it.value()->status() == SearchStatus::Searching) {
            return true;
        }
    }
    
    return false;
}

QList<SearchResult> SearchManager::results() const
{
    return d->searchResults;
}

void SearchManager::setResultHandler(std::function<void(const SearchResult&)> callback)
{
    d->resultHandler = callback;
}

void SearchManager::clearResults()
{
    d->searchResults.clear();
}

} // namespace DFMSearch 