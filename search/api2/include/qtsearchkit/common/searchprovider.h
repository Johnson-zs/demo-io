#pragma once

#include <QObject>
#include <QString>
#include <QFuture>
#include <functional>
#include <memory>
#include <set>
#include "../search/searchquery.h"
#include "../search/searchresult.h"
#include "searchfeatures.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchProvider : public QObject {
    Q_OBJECT
public:
    explicit SearchProvider(QObject* parent = nullptr);
    ~SearchProvider() override;
    
    // 提供商标识和名称
    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    
    // 支持的特性
    virtual SearchFeatures features() const = 0;
    
    // 主要搜索功能
    virtual QFuture<std::shared_ptr<SearchResultSet>> search(
        const SearchQuery& query, 
        SearchType type) = 0;
    
    // 支持增量结果的搜索
    using ResultCallback = std::function<void(const SearchResultItem&)>;
    virtual QFuture<std::shared_ptr<SearchResultSet>> searchWithCallback(
        const SearchQuery& query,
        const ResultCallback& callback,
        SearchType type);
    
    // 取消搜索操作
    virtual void cancelSearch() = 0;
    
    // 查询支持的选项和模式
    bool supportsOption(const QString& option) const;
    bool supportsMode(SearchMode mode) const;
    bool supportsType(SearchType type) const;
    
    // 功能查询
    virtual bool supportsIncremental() const;
    virtual bool requiresIndexing() const;
    
    // 关联的索引管理器(如果适用)
    virtual QString associatedIndexManagerId() const;
    
    // 验证查询是否有效
    virtual bool validateQuery(const SearchQuery& query, SearchType type, QString* errorMessage = nullptr) const;
    
signals:
    void searchError(const QString& error);
};

} // namespace QtSearchKit 