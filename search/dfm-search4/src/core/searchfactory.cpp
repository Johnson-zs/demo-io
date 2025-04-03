#include <dfm6-search/searchfactory.h>
#include "../filenamesearch/filenamesearchengine.h"
#include "../contentsearch/contentsearchengine.h"
#include "../filenamesearch/filenamesearchoptions.h"
#include "../contentsearch/contentsearchoptions.h"
#include <QMutex>
#include <QMap>

namespace DFM6 {
namespace Search {

class SearchFactoryData
{
public:
    QMutex mutex;
    QMap<QString, std::shared_ptr<SearchProvider>> providers;
};

SearchFactory& SearchFactory::instance()
{
    static SearchFactory factory;
    return factory;
}

SearchFactory::SearchFactory()
    : d(std::make_unique<SearchFactoryData>())
{
}

SearchFactory::~SearchFactory() = default;

SearchEngine* SearchFactory::createEngine(SearchType type, QObject *parent)
{
    SearchEngine* engine = nullptr;
    
    switch (type) {
    case SearchType::FileName:
        engine = new SearchEngine(type, parent);
        break;
    case SearchType::Content:
        engine = new SearchEngine(type, parent);
        break;
    case SearchType::Custom:
        // 由应用程序基于provider自行创建
        break;
    }
    
    return engine;
}

bool SearchFactory::registerProvider(std::shared_ptr<SearchProvider> provider)
{
    if (!provider) {
        return false;
    }
    
    QMutexLocker locker(&d->mutex);
    if (d->providers.contains(provider->id())) {
        return false;  // 已存在同ID的提供者
    }
    
    d->providers[provider->id()] = provider;
    return true;
}

bool SearchFactory::unregisterProvider(const QString &providerId)
{
    QMutexLocker locker(&d->mutex);
    return d->providers.remove(providerId) > 0;
}

QList<std::shared_ptr<SearchProvider>> SearchFactory::providers() const
{
    QMutexLocker locker(&d->mutex);
    return d->providers.values();
}

std::shared_ptr<SearchProvider> SearchFactory::provider(const QString &providerId) const
{
    QMutexLocker locker(&d->mutex);
    return d->providers.value(providerId);
}

bool SearchFactory::hasProviderForType(SearchType type) const
{
    QMutexLocker locker(&d->mutex);
    for (const auto &provider : d->providers) {
        if (provider->supportsType(type)) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<SearchOptions> SearchFactory::createOptions(SearchType type)
{
    switch (type) {
    case SearchType::FileName:
        return std::make_unique<FileNameSearchOptions>();
    case SearchType::Content:
        return std::make_unique<ContentSearchOptions>();
    case SearchType::Custom:
        return std::make_unique<SearchOptions>();
    default:
        return std::make_unique<SearchOptions>();
    }
}

SearchQuery SearchFactory::createQuery(const QString &keyword, QueryType type)
{
    return SearchQuery(keyword, type);
}

void SearchFactory::registerEngineCreator(SearchType type, 
                                         std::function<std::shared_ptr<AbstractSearchEngine>(QObject*)> creator)
{
    // 这里可以添加自定义引擎创建器的注册逻辑
    // 需要在SearchFactoryData中添加相应的存储
}

std::unique_ptr<SearchResult> SearchFactory::createResult(SearchType type, const QString &path)
{
    std::unique_ptr<SearchResult> result = std::make_unique<SearchResult>();
    result->setPath(path);
    result->setType(type);
    // 根据类型设置其他属性
    return result;
}

}  // namespace Search
}  // namespace DFM6 