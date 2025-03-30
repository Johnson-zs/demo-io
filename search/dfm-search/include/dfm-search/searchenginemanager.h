#ifndef DFM_SEARCH_ENGINE_MANAGER_H
#define DFM_SEARCH_ENGINE_MANAGER_H

#include "searchengine.h"
#include <vector>
#include <memory>

namespace DFM {
namespace Search {

/**
 * @brief 搜索引擎管理器类，用于创建和管理搜索引擎
 */
class SearchEngineManager : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static SearchEngineManager* instance();

    // 构造函数和析构函数
    explicit SearchEngineManager(QObject* parent = nullptr);
    ~SearchEngineManager();

    // 引擎注册和管理
    void registerEngine(std::shared_ptr<ISearchEngine> engine);
    void unregisterEngine(std::shared_ptr<ISearchEngine> engine);
    std::shared_ptr<ISearchEngine> createEngine(SearchType type, SearchMode mode);
    const std::vector<std::shared_ptr<ISearchEngine>>& engines() const;

private:
    // 注册默认搜索引擎
    void registerDefaultEngines();

    // 搜索引擎列表
    std::vector<std::shared_ptr<ISearchEngine>> m_engines;

signals:
    void engineAdded(const std::shared_ptr<DFM::Search::ISearchEngine>& engine);
    void engineRemoved(const std::shared_ptr<DFM::Search::ISearchEngine>& engine);
    void resultsReady(SearchType type, const DFM::Search::SearchResultSet& results);
    void searchCompleted(SearchType type, bool success);
    void errorOccurred(SearchType type, const QString& error);
};

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_ENGINE_MANAGER_H 