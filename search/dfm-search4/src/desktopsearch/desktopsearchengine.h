#ifndef DESKTOP_SEARCH_ENGINE_H
#define DESKTOP_SEARCH_ENGINE_H

#include "../core/abstractsearchengine.h"
#include "desktopsearchoptions.h"
#include <memory>
#include <QFileInfo>

namespace DFM6 {
namespace Search {

/**
 * @brief 桌面应用搜索引擎
 *
 * 实现基于Desktop Entry规范的应用搜索功能
 */
class DesktopSearchEngine : public AbstractSearchEngine
{
    Q_OBJECT

public:
    explicit DesktopSearchEngine(QObject *parent = nullptr);
    ~DesktopSearchEngine() override;
    
    // 实现AbstractSearchEngine接口
    SearchType searchType() const override { return SearchType::Desktop; }
    void setSearchType(SearchType) override {} // 类型固定为Desktop
    
    SearchOptions searchOptions() const override;
    void setSearchOptions(const SearchOptions &options) override;
    
    SearchStatus status() const override;
    
    QFuture<QList<SearchResult>> search(const SearchQuery &query) override;
    QFuture<void> searchWithCallback(const SearchQuery &query, 
                                   SearchEngine::ResultCallback callback) override;
    QList<SearchResult> searchSync(const SearchQuery &query) override;
    
    void pause() override;
    void resume() override;
    void cancel() override;
    void clearCache() override;

private:
    // 解析.desktop文件
    SearchResult parseDesktopFile(const QString &filePath, const SearchQuery &query);
    
    // 检查desktop文件是否符合查询条件
    bool matchesQuery(const QFileInfo &fileInfo, const QMap<QString, QString> &desktopData, const SearchQuery &query);
    
    // 计算搜索匹配得分
    float calculateScore(const QMap<QString, QString> &desktopData, const SearchQuery &query);
    
    // 处理暂停
    void processPauseIfNeeded();
    
    DesktopSearchOptions m_options;
    QStringList m_standardPaths; // 标准desktop文件路径
};

}  // namespace Search
}  // namespace DFM6

#endif // DESKTOP_SEARCH_ENGINE_H 