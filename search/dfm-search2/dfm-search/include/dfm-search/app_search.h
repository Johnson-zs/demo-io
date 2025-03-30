#ifndef DFM_APP_SEARCH_H
#define DFM_APP_SEARCH_H

#include <memory>
#include <vector>
#include <functional>

#include <QString>
#include <QStringList>
#include <QIcon>
#include <QMap>
#include <QVariant>

#include "search_engine.h" // 引入SearchResult和相关结构

namespace DFM {
namespace Search {

/**
 * @brief 应用程序信息类
 * 
 * 包含桌面应用程序的基本信息
 */
class AppInfo {
public:
    QString appId;             ///< 应用程序ID
    QString name;              ///< 应用程序名称
    QString displayName;       ///< 显示名称（本地化）
    QString description;       ///< 应用描述
    QString desktopFilePath;   ///< .desktop文件路径
    QString iconName;          ///< 图标名称
    QString execCommand;       ///< 执行命令
    QStringList categories;    ///< 应用类别
    QStringList keywords;      ///< 关键词
    bool hidden{false};        ///< 是否隐藏
    QMap<QString, QString> localizedNames; ///< 本地化名称
    
    // 自定义元数据
    QVariantMap metadata;
    
    // 方法
    QIcon icon() const;
    bool launch() const;
    bool launchWithArgs(const QStringList& args) const;
};

/**
 * @brief 应用程序类别枚举
 */
enum class AppCategory {
    All,
    Accessories,
    Development,
    Education,
    Games,
    Graphics,
    Internet,
    Multimedia,
    Office,
    Science,
    Settings,
    System,
    Utility,
    Other
};

/**
 * @brief 应用程序索引配置
 */
class AppIndexConfig {
public:
    QStringList additionalAppDirs;        ///< 额外应用目录，默认会搜索标准XDG目录
    bool indexUserApps{true};             ///< 是否索引用户应用程序目录
    bool indexSystemApps{true};           ///< 是否索引系统应用程序目录
    QStringList excludeCategories;        ///< 排除的应用类别
    bool includeHidden{false};            ///< 是否包含隐藏应用
    int updateIntervalMinutes{60};        ///< 更新间隔（分钟）
};

/**
 * @brief 应用程序搜索引擎
 * 
 * 专门用于搜索桌面应用程序
 */
class AppSearchEngine : public QObject {
    Q_OBJECT
    
public:
    // PIMPL模式
    class Impl;
    
    AppSearchEngine();
    virtual ~AppSearchEngine();
    
    // 初始化和索引操作
    bool initialize(const AppIndexConfig& config = AppIndexConfig());
    bool update();
    bool rebuild();
    
    // 搜索操作
    std::vector<AppInfo> search(const QString& query, 
                                int maxResults = 10, 
                                AppCategory category = AppCategory::All,
                                bool caseSensitive = false) const;
    
    // 应用检索方法
    AppInfo getAppByDesktopFile(const QString& desktopFilePath) const;
    AppInfo getAppById(const QString& appId) const;
    std::vector<AppInfo> getAppsByCategory(AppCategory category) const;
    
    // 其他查询方法
    QStringList getAllCategories() const;
    int getAppCount() const;
    
signals:
    void indexUpdated();
    void searchCompleted(const std::vector<DFM::Search::AppInfo>& results);
    
private:
    std::unique_ptr<Impl> d;
};

/**
 * @brief 将AppCategory转换为字符串
 */
QString appCategoryToString(AppCategory category);

/**
 * @brief 将字符串转换为AppCategory
 */
AppCategory stringToAppCategory(const QString& categoryStr);

} // namespace Search
} // namespace DFM

#endif // DFM_APP_SEARCH_H 