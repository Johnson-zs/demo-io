#include "dfm-search/searchenginemanager.h"
#include "dfm-search/filenamesearch.h"
#include "dfm-search/contentsearch.h"
#include <algorithm>

namespace DFM {
namespace Search {

// 初始化静态成员
std::map<SearchEngineFactory::EngineKey, SearchEngineFactory::EngineCreator> SearchEngineFactory::s_registry;

// 实现CreateEngine静态方法
std::shared_ptr<ISearchEngine> SearchEngineFactory::createEngine(SearchType type, SearchMode mode, QObject* parent)
{
    auto key = std::make_pair(type, mode);
    auto it = s_registry.find(key);
    if (it != s_registry.end()) {
        return it->second(parent);
    }
    return nullptr;
}

SearchEngineManager* SearchEngineManager::instance()
{
    static SearchEngineManager instance;
    return &instance;
}

SearchEngineManager::SearchEngineManager(QObject* parent)
    : QObject(parent)
{
    // 注册默认的搜索引擎
    registerDefaultEngines();
}

SearchEngineManager::~SearchEngineManager()
{
    m_engines.clear();
}

void SearchEngineManager::registerDefaultEngines()
{
    // 注册文件名搜索引擎
    registerEngine(std::make_shared<RealtimeFilenameSearchEngine>());
    registerEngine(std::make_shared<IndexedFilenameSearchEngine>());
    
    // 注册内容搜索引擎
    registerEngine(std::make_shared<RealtimeContentSearchEngine>());
    registerEngine(std::make_shared<IndexedContentSearchEngine>());
}

void SearchEngineManager::registerEngine(std::shared_ptr<ISearchEngine> engine)
{
    if (!engine) {
        return;
    }
    
    // 连接引擎信号
    connect(engine.get(), &ISearchEngine::resultsReady,
            this, [this, engine](const SearchResultSet& results) {
                emit resultsReady(engine->supportedType(), results);
            });
            
    connect(engine.get(), &ISearchEngine::searchCompleted,
            this, [this, engine](bool success) {
                emit searchCompleted(engine->supportedType(), success);
            });
            
    connect(engine.get(), &ISearchEngine::errorOccurred,
            this, [this, engine](const QString& error) {
                emit errorOccurred(engine->supportedType(), error);
            });
    
    // 添加到引擎列表
    m_engines.push_back(engine);
    emit engineAdded(engine);
}

void SearchEngineManager::unregisterEngine(std::shared_ptr<ISearchEngine> engine)
{
    if (!engine) {
        return;
    }
    
    auto it = std::find(m_engines.begin(), m_engines.end(), engine);
    if (it != m_engines.end()) {
        m_engines.erase(it);
        emit engineRemoved(engine);
    }
}

std::shared_ptr<ISearchEngine> SearchEngineManager::createEngine(SearchType type, SearchMode mode)
{
    // 根据类型和模式创建对应的搜索引擎
    std::shared_ptr<ISearchEngine> engine;
    
    switch (type) {
    case SearchType::FileName:
        if (mode == SearchMode::Realtime) {
            engine = std::make_shared<RealtimeFilenameSearchEngine>();
        } else {
            engine = std::make_shared<IndexedFilenameSearchEngine>();
        }
        break;
        
    case SearchType::Fulltext:
        if (mode == SearchMode::Realtime) {
            engine = std::make_shared<RealtimeContentSearchEngine>();
        } else {
            engine = std::make_shared<IndexedContentSearchEngine>();
        }
        break;
        
    default:
        break;
    }
    
    if (engine) {
        registerEngine(engine);
    }
    
    return engine;
}

const std::vector<std::shared_ptr<ISearchEngine>>& SearchEngineManager::engines() const
{
    return m_engines;
}

bool SearchEngineManager::search(const QString& query)
{
    bool result = false;
    
    // 对所有引擎执行搜索
    for (const auto& engine : m_engines) {
        if (engine->startSearch(query)) {
            result = true;
        }
    }
    
    return result;
}

void SearchEngineManager::pauseAll()
{
    for (const auto& engine : m_engines) {
        engine->pauseSearch();
    }
}

void SearchEngineManager::resumeAll()
{
    for (const auto& engine : m_engines) {
        engine->resumeSearch();
    }
}

void SearchEngineManager::cancelAll()
{
    for (const auto& engine : m_engines) {
        engine->cancelSearch();
    }
}

void SearchEngineManager::onEngineStateChanged(SearchState state)
{
    // 可以在这里添加状态变化处理逻辑
    Q_UNUSED(state)
}

void SearchEngineManager::onEngineResultsReady(const SearchResultSet& results)
{
    // 获取发送信号的引擎
    auto sender = qobject_cast<ISearchEngine*>(QObject::sender());
    if (!sender) {
        return;
    }
    
    // 发送结果就绪信号，包含引擎类型
    emit resultsReady(sender->supportedType(), results);
}

void SearchEngineManager::onEngineSearchCompleted(bool success)
{
    // 获取发送信号的引擎
    auto sender = qobject_cast<ISearchEngine*>(QObject::sender());
    if (!sender) {
        return;
    }
    
    // 发送搜索完成信号，包含引擎类型
    emit searchCompleted(sender->supportedType(), success);
}

void SearchEngineManager::onEngineErrorOccurred(const QString& error)
{
    // 获取发送信号的引擎
    auto sender = qobject_cast<ISearchEngine*>(QObject::sender());
    if (!sender) {
        return;
    }
    
    // 发送错误信号，包含引擎类型
    emit errorOccurred(sender->supportedType(), error);
}

} // namespace Search
} // namespace DFM 
