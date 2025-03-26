#pragma once

#include <QObject>
#include <QFuture>
#include <functional>
#include "searchquery.h"
#include "searchresult.h"
#include "../common/searchprovider.h"
#include "../common/searchfeatures.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchEngine : public QObject {
    Q_OBJECT
public:
    explicit SearchEngine(QObject* parent = nullptr);
    ~SearchEngine() override;
    
    // 提供商管理
    bool registerProvider(const QString& id, std::shared_ptr<SearchProvider> provider);
    bool unregisterProvider(const QString& id);
    std::shared_ptr<SearchProvider> provider(const QString& id) const;
    QStringList availableProviders() const;
    
    // 搜索类型管理
    QList<SearchType> supportedSearchTypes() const;
    QStringList providersForType(SearchType type) const;
    
    // 查询系统能力
    SearchFeatures featuresForType(SearchType type) const;
    SearchFeatures featuresForProvider(const QString& providerId) const;
    
    // 主要的搜索函数
    QFuture<std::shared_ptr<SearchResultSet>> search(
        const SearchQuery& query,
        const QStringList& providerIds = QStringList(),
        SearchType type = SearchType::FileName);
    
    // 支持增量结果
    using ResultCallback = std::function<void(const SearchResultItem&)>;
    QFuture<std::shared_ptr<SearchResultSet>> searchWithCallback(
        const SearchQuery& query,
        const ResultCallback& callback,
        const QStringList& providerIds = QStringList(),
        SearchType type = SearchType::FileName);
        
    // 取消搜索
    void cancelSearch();
    bool isSearching() const;
    
    // 查询构建助手
    SearchQuery createQueryTemplate(SearchType type) const;
    bool validateQuery(const SearchQuery& query, 
                      SearchType type, 
                      const QStringList& providerIds = QStringList(),
                      QString* errorMessage = nullptr) const;

signals:
    void searchStarted();
    void searchFinished();
    void searchCancelled();
    void searchError(const QString& error);
    void providerAdded(const QString& id);
    void providerRemoved(const QString& id);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 