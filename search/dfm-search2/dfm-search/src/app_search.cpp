#include <dfm-search/app_search.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

namespace DFM {
namespace Search {

// AppSearchEngine私有实现
class AppSearchEngine::Impl {
public:
    Impl(AppSearchEngine* q)
        : q_ptr(q)
        , initialized(false)
    {
    }
    
    // 引用外部类
    AppSearchEngine* q_ptr;
    
    // 状态相关
    bool initialized;
    AppIndexConfig config;
    std::vector<AppInfo> appList;
    
    // 互斥锁
    mutable QMutex mutex;
    
    // 初始化应用列表 - 实际中应该读取.desktop文件
    void initializeAppList() {
        // 模拟添加一些应用
        appList.clear();
        
        // 添加模拟应用1
        AppInfo app1;
        app1.setAppId("org.kde.dolphin");
        app1.setName("dolphin");
        app1.setDisplayName("Dolphin");
        app1.setDescription("KDE 文件管理器");
        app1.setDesktopFilePath("/usr/share/applications/org.kde.dolphin.desktop");
        app1.setIconName("system-file-manager");
        app1.setExecCommand("dolphin");
        app1.metadata().insert("Category", "FileManager");
        appList.push_back(app1);
        
        // 添加模拟应用2
        AppInfo app2;
        app2.setAppId("org.gnome.Terminal");
        app2.setName("gnome-terminal");
        app2.setDisplayName("终端");
        app2.setDescription("GNOME 终端模拟器");
        app2.setDesktopFilePath("/usr/share/applications/org.gnome.Terminal.desktop");
        app2.setIconName("utilities-terminal");
        app2.setExecCommand("gnome-terminal");
        app2.metadata().insert("Category", "System");
        appList.push_back(app2);
    }
    
    // 搜索应用 - 简单实现
    std::vector<AppInfo> searchApps(const QString& query, int maxResults, AppCategory category) const {
        if (query.isEmpty()) {
            return appList;
        }
        
        std::vector<AppInfo> results;
        QString queryLower = query.toLower();
        
        for (const auto& app : appList) {
            // 简单的字符串匹配
            if (app.name().toLower().contains(queryLower) || 
                app.displayName().toLower().contains(queryLower)) {
                results.push_back(app);
                
                if (static_cast<int>(results.size()) >= maxResults) {
                    break;
                }
            }
        }
        
        return results;
    }
};

// AppInfo实现
QIcon AppInfo::icon() const {
    return QIcon::fromTheme(iconName);
}

bool AppInfo::launch() const {
    return launchWithArgs(QStringList());
}

bool AppInfo::launchWithArgs(const QStringList& args) const {
    if (execCommand.isEmpty()) {
        return false;
    }
    
    QString cmd = execCommand;
    return QProcess::startDetached(cmd, args);
}

// AppSearchEngine构造函数
AppSearchEngine::AppSearchEngine()
    : d(new Impl(this))
{
}

// AppSearchEngine析构函数
AppSearchEngine::~AppSearchEngine() = default;

// 初始化
bool AppSearchEngine::initialize(const AppIndexConfig& config)
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        qWarning() << "应用搜索引擎已经初始化";
        return true; // 已经初始化，视为成功
    }
    
    d->config = config;
    
    // 初始化应用列表
    d->initializeAppList();
    
    d->initialized = true;
    emit indexUpdated();
    
    return true;
}

// 更新索引
bool AppSearchEngine::update()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "应用搜索引擎未初始化";
        return false;
    }
    
    // 重新初始化应用列表
    d->initializeAppList();
    
    emit indexUpdated();
    
    return true;
}

// 重建索引
bool AppSearchEngine::rebuild()
{
    return update();
}

// 搜索应用
std::vector<AppInfo> AppSearchEngine::search(const QString& query, 
                                              int maxResults, 
                                              AppCategory category,
                                              bool caseSensitive) const
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        qWarning() << "应用搜索引擎未初始化";
        return {};
    }
    
    auto results = d->searchApps(query, maxResults, category);
    
    emit const_cast<AppSearchEngine*>(this)->searchCompleted(results);
    
    return results;
}

// 根据桌面文件路径获取应用
AppInfo AppSearchEngine::getAppByDesktopFile(const QString& desktopFilePath) const
{
    QMutexLocker locker(&d->mutex);
    
    for (const auto& app : d->appList) {
        if (app.desktopFilePath() == desktopFilePath) {
            return app;
        }
    }
    
    return AppInfo();
}

// 根据应用ID获取应用
AppInfo AppSearchEngine::getAppById(const QString& appId) const
{
    QMutexLocker locker(&d->mutex);
    
    for (const auto& app : d->appList) {
        if (app.appId() == appId) {
            return app;
        }
    }
    
    return AppInfo();
}

// 根据类别获取应用列表
std::vector<AppInfo> AppSearchEngine::getAppsByCategory(AppCategory category) const
{
    QMutexLocker locker(&d->mutex);
    
    if (category == AppCategory::All) {
        return d->appList;
    }
    
    std::vector<AppInfo> results;
    QString categoryStr = appCategoryToString(category);
    
    for (const auto& app : d->appList) {
        QVariant categoryVar = app.metadata().value("Category");
        if (categoryVar.isValid() && categoryVar.toString() == categoryStr) {
            results.push_back(app);
        }
    }
    
    return results;
}

// 获取所有类别
QStringList AppSearchEngine::getAllCategories() const
{
    QMutexLocker locker(&d->mutex);
    
    QStringList categories;
    for (const auto& app : d->appList) {
        QVariant categoryVar = app.metadata().value("Category");
        if (categoryVar.isValid()) {
            QString category = categoryVar.toString();
            if (!categories.contains(category)) {
                categories.append(category);
            }
        }
    }
    
    return categories;
}

// 获取应用总数
int AppSearchEngine::getAppCount() const
{
    QMutexLocker locker(&d->mutex);
    return static_cast<int>(d->appList.size());
}

// 将类别枚举转换为字符串
QString appCategoryToString(AppCategory category)
{
    switch (category) {
        case AppCategory::Accessories: return "Accessories";
        case AppCategory::Development: return "Development";
        case AppCategory::Education: return "Education";
        case AppCategory::Games: return "Games";
        case AppCategory::Graphics: return "Graphics";
        case AppCategory::Internet: return "Internet";
        case AppCategory::Multimedia: return "Multimedia";
        case AppCategory::Office: return "Office";
        case AppCategory::Science: return "Science";
        case AppCategory::Settings: return "Settings";
        case AppCategory::System: return "System";
        case AppCategory::Utility: return "Utility";
        case AppCategory::All:
        case AppCategory::Other:
        default: return "Other";
    }
}

// 将字符串转换为类别枚举
AppCategory stringToAppCategory(const QString& categoryStr)
{
    if (categoryStr == "Accessories") return AppCategory::Accessories;
    if (categoryStr == "Development") return AppCategory::Development;
    if (categoryStr == "Education") return AppCategory::Education;
    if (categoryStr == "Games") return AppCategory::Games;
    if (categoryStr == "Graphics") return AppCategory::Graphics;
    if (categoryStr == "Internet") return AppCategory::Internet;
    if (categoryStr == "Multimedia") return AppCategory::Multimedia;
    if (categoryStr == "Office") return AppCategory::Office;
    if (categoryStr == "Science") return AppCategory::Science;
    if (categoryStr == "Settings") return AppCategory::Settings;
    if (categoryStr == "System") return AppCategory::System;
    if (categoryStr == "Utility") return AppCategory::Utility;
    return AppCategory::Other;
}

} // namespace Search
} // namespace DFM 