#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QIcon>
#include "../common/searchprovider.h"
#include "../search/searchquery.h"
#include "../search/searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 应用信息结构
class QTSEARCHKIT_EXPORT AppInfo {
public:
    QString id() const;
    void setId(const QString& id);
    
    QString name() const;
    void setName(const QString& name);
    
    QString displayName() const;
    void setDisplayName(const QString& name);
    
    QString description() const;
    void setDescription(const QString& desc);
    
    QString executable() const;
    void setExecutable(const QString& exec);
    
    QString desktopFilePath() const;
    void setDesktopFilePath(const QString& path);
    
    QStringList categories() const;
    void setCategories(const QStringList& categories);
    
    QStringList keywords() const;
    void setKeywords(const QStringList& keywords);
    
    QString iconName() const;
    void setIconName(const QString& name);
    
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    
    bool isHidden() const;
    void setHidden(bool hidden);
    
    // 转换为SearchResultItem
    SearchResultItem toSearchResultItem() const;
    
    // 从desktop文件加载
    static AppInfo fromDesktopFile(const QString& path);
    
private:
    QString m_id;
    QString m_name;
    QString m_displayName;
    QString m_description;
    QString m_executable;
    QString m_desktopFilePath;
    QStringList m_categories;
    QStringList m_keywords;
    QString m_iconName;
    QIcon m_icon;
    bool m_hidden = false;
};

// 应用搜索提供商
class QTSEARCHKIT_EXPORT AppSearchProvider : public SearchProvider {
    Q_OBJECT
public:
    explicit AppSearchProvider(QObject* parent = nullptr);
    ~AppSearchProvider() override;
    
    // SearchProvider接口实现
    QString id() const override;
    QString displayName() const override;
    
    SearchFeatures features() const override;
    
    QFuture<std::shared_ptr<SearchResultSet>> search(
        const SearchQuery& query, 
        SearchType type) override;
    
    void cancelSearch() override;
    
    bool supportsIncremental() const override;
    bool requiresIndexing() const override;
    
    // 应用搜索特定功能
    void setAppDirectories(const QStringList& dirs);
    QStringList appDirectories() const;
    
    void setFollowXdgDataDirs(bool follow);
    bool followXdgDataDirs() const;
    
    // 扫描和索引应用
    QFuture<bool> scanApplications();
    int appCount() const;
    
    // 应用管理
    QList<AppInfo> allApps() const;
    AppInfo appById(const QString& id) const;
    
    // 启动应用
    bool launchApp(const QString& appId) const;
    
signals:
    void applicationScanned(const QString& id, const QString& name);
    void scanningFinished(int count);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 