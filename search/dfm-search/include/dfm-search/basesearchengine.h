#ifndef DFM_BASE_SEARCH_ENGINE_H
#define DFM_BASE_SEARCH_ENGINE_H

#include "searchengine.h"

namespace DFM {
namespace Search {

/**
 * @brief 搜索引擎基类，提供共同功能的实现
 */
class BaseSearchEngine : public ISearchEngine {
    Q_OBJECT

public:
    explicit BaseSearchEngine(QObject* parent = nullptr);
    virtual ~BaseSearchEngine() = default;

    // ISearchEngine接口实现
    void pauseSearch() override;
    void resumeSearch() override;
    void cancelSearch() override;
    SearchState state() const override;
    QString lastError() const override;

protected:
    // 保护方法
    void setState(SearchState state) override;
    void setError(const QString& error) override;
    void updateProgress(const SearchProgress& progress);
    void addResults(const SearchResultSet& results);

    // 成员变量
    SearchState m_state;
    QString m_lastError;
};

} // namespace Search
} // namespace DFM

#endif // DFM_BASE_SEARCH_ENGINE_H 