#include "qsearch/engines/engine_factory.h"
#include "qsearch/engines/filename_engine.h"
#include "qsearch/engines/fulltext_engine.h"
#include "qsearch/engines/appdesktop_engine.h"
#include <QMutex>
#include <QMutexLocker>

namespace QSearch {

struct EngineFactory::Impl {
    QMap<QString, std::function<SearchEngine*(QObject*)>> engineFactories;
    QMutex mutex;
    
    Impl() {
        // 注册内置搜索引擎
        engineFactories["filename"] = [](QObject* parent) -> SearchEngine* { 
            return new FilenameSearchEngine(parent);
        };
        
        engineFactories["fulltext"] = [](QObject* parent) -> SearchEngine* { 
            return new FulltextSearchEngine(parent);
        };
        
        // 注册应用搜索引擎
        engineFactories["appdesktop"] = [](QObject* parent) -> SearchEngine* {
            return new AppDesktopSearchEngine(parent);
        };
    }
};

// 单例实现
EngineFactory& EngineFactory::instance() {
    static EngineFactory instance;
    return instance;
}

EngineFactory::EngineFactory(QObject* parent) : QObject(parent), d(new Impl) {
}

EngineFactory::~EngineFactory() {
}

QStringList EngineFactory::availableEngines() const {
    QMutexLocker locker(&d->mutex);
    return d->engineFactories.keys();
}

QSharedPointer<SearchEngine> EngineFactory::createEngine(const QString& engineId) {
    QMutexLocker locker(&d->mutex);
    
    if (!d->engineFactories.contains(engineId)) {
        return nullptr;
    }
    
    SearchEngine* engine = d->engineFactories[engineId](this);
    return QSharedPointer<SearchEngine>(engine);
}

QSharedPointer<SearchEngine> EngineFactory::createEngineForQuery(const SearchQuery& query) {
    // 根据查询类型选择合适的引擎
    switch (query.type()) {
        case QueryType::Filename:
            return createEngine("filename");
        case QueryType::FileContent:
            return createEngine("fulltext");
        case QueryType::Application:
            return createEngine("appdesktop");
        case QueryType::Both: {
            // 对于复合查询，选择支持的引擎
            if (query.matchType() == MatchType::Regex || 
                query.matchType() == MatchType::Exact) {
                return createEngine("fulltext");
            } else {
                return createEngine("filename");
            }
        }
        case QueryType::Extended:
            // 从扩展引擎中选择
            // 这里简化处理，返回文件名引擎
            return createEngine("filename");
        default:
            return createEngine("filename");
    }
}

bool EngineFactory::registerEngine(const QString& engineId, 
                                 std::function<SearchEngine*(QObject*)> factory) {
    QMutexLocker locker(&d->mutex);
    
    if (d->engineFactories.contains(engineId)) {
        return false;
    }
    
    d->engineFactories[engineId] = factory;
    return true;
}

bool EngineFactory::unregisterEngine(const QString& engineId) {
    QMutexLocker locker(&d->mutex);
    
    if (!d->engineFactories.contains(engineId)) {
        return false;
    }
    
    d->engineFactories.remove(engineId);
    return true;
}

} // namespace QSearch 