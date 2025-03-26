#pragma once

#include <QObject>
#include <QString>
#include "../search/searchquery.h"
#include "../search/searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchAccessManager : public QObject {
    Q_OBJECT
public:
    explicit SearchAccessManager(QObject* parent = nullptr);
    
    // 访问规则管理
    void addAccessRule(const QString& pattern, bool allow);
    void removeAccessRule(const QString& pattern);
    QStringList accessRules() const;
    
    // 当前用户信息
    void setCurrentUser(const QString& username);
    QString currentUser() const;
    
    void setUserGroups(const QStringList& groups);
    QStringList userGroups() const;
    
    // 权限检查
    bool canAccess(const QString& path) const;
    bool canAccess(const SearchResultItem& item) const;
    
    // 应用权限过滤
    void filterResults(std::shared_ptr<SearchResultSet> results);
    
    // 自动应用权限过滤到查询
    void applyRestrictions(SearchQuery& query);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 