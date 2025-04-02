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

std::unique_ptr<SearchEngine> SearchFactory::createEngine(SearchType type)
{
    std::unique_ptr<SearchEngine> engine;
    
    switch (type) {
    case SearchType::FileName:
        engine = std::make_unique<SearchEngine>(type);
        break;
    case SearchType::Content:
        engine = std::make_unique<SearchEngine>(type);
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

}  // namespace Search
}  // namespace DFM6 