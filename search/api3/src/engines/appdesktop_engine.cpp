#include "qsearch/engines/appdesktop_engine.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QTimer>
#include <QLocale>
#include <QFileInfo>
#include <QStandardPaths>
#include <atomic>

namespace QSearch {

struct AppDesktopSearchEngine::Impl {
    SearchQuery query;
    SearchOptions options;
    SearchState currentState = SearchState::Idle;
    SearchResult results;
    std::atomic<double> progress{0.0};
    QTimer* progressTimer = nullptr;
    bool stopRequested = false;
    
    // 应用信息缓存
    QList<AppInfo> appInfoCache;
    
    // 构造函数
    Impl() : progressTimer(new QTimer) {
    }
    
    // 析构函数
    ~Impl() {
        progressTimer->stop();
        delete progressTimer;
    }
    
    // 刷新应用缓存
    void refreshAppCache() {
        appInfoCache.clear();
        
        // 搜索标准目录中的.desktop文件
        QStringList desktopDirs = {
            "/usr/share/applications/",
            "/usr/local/share/applications/",
            QDir::homePath() + "/.local/share/applications/"
        };
        
        // 添加XDG标准路径
        desktopDirs.append(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation));
        
        // 遍历目录查找.desktop文件
        for (const QString& dir : desktopDirs) {
            QDir applicationsDir(dir);
            if (!applicationsDir.exists()) continue;
            
            QStringList desktopFiles = applicationsDir.entryList(QStringList() << "*.desktop", QDir::Files);
            for (const QString& file : desktopFiles) {
                QString desktopFilePath = applicationsDir.filePath(file);
                
                // 解析.desktop文件
                AppInfo appInfo = parseDesktopFile(desktopFilePath);
                if (!appInfo.name.isEmpty() && !appInfo.noDisplay && !appInfo.hidden) {
                    appInfoCache.append(appInfo);
                }
            }
        }
    }
    
    // 解析.desktop文件
    AppInfo parseDesktopFile(const QString& filePath) {
        QSettings settings(filePath, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.beginGroup("Desktop Entry");
        
        AppInfo info;
        info.desktopFile = filePath;
        
        // 获取当前语言的本地化名称
        QString locale = QLocale::system().name();
        if (settings.contains("Name[" + locale + "]")) {
            info.name = settings.value("Name[" + locale + "]").toString();
        } else {
            info.name = settings.value("Name").toString();
        }
        
        // 获取其他属性
        if (settings.contains("GenericName[" + locale + "]")) {
            info.genericName = settings.value("GenericName[" + locale + "]").toString();
        } else {
            info.genericName = settings.value("GenericName").toString();
        }
        
        if (settings.contains("Comment[" + locale + "]")) {
            info.comment = settings.value("Comment[" + locale + "]").toString();
        } else {
            info.comment = settings.value("Comment").toString();
        }
        
        info.exec = settings.value("Exec").toString();
        info.icon = settings.value("Icon").toString();
        info.categories = settings.value("Categories").toString().split(';', Qt::SkipEmptyParts);
        info.keywords = settings.value("Keywords").toString().split(';', Qt::SkipEmptyParts);
        info.hidden = settings.value("Hidden", false).toBool();
        info.noDisplay = settings.value("NoDisplay", false).toBool();
        
        settings.endGroup();
        return info;
    }
    
    // 搜索应用
    bool searchApps() {
        // 如果缓存为空，先刷新缓存
        if (appInfoCache.isEmpty()) {
            refreshAppCache();
        }
        
        // 获取搜索关键词
        QString searchText = query.text().toLower();
        if (searchText.isEmpty()) {
            // 空搜索返回所有应用
            for (const AppInfo& app : appInfoCache) {
                addAppToResults(app);
            }
            return true;
        }
        
        // 计算每个应用的相关性得分并添加匹配的应用到结果
        for (AppInfo app : appInfoCache) {
            // 计算相关性得分
            double score = calculateRelevance(app, searchText);
            
            // 如果得分大于0，添加到结果中
            if (score > 0) {
                app.relevanceScore = score;
                addAppToResults(app);
            }
            
            // 检查是否被请求停止
            if (stopRequested) {
                return false;
            }
        }
        
        // 根据相关性得分对结果排序
        results.sort(SortField::Relevance, SortOrder::Descending);
        
        return true;
    }
    
    // 计算应用与搜索词的相关性得分
    double calculateRelevance(const AppInfo& app, const QString& searchText) {
        double score = 0.0;
        
        // 名称完全匹配得高分
        if (app.name.toLower() == searchText) {
            score += 100.0;
        }
        // 名称开头匹配
        else if (app.name.toLower().startsWith(searchText)) {
            score += 80.0;
        }
        // 名称包含搜索词
        else if (app.name.toLower().contains(searchText)) {
            score += 60.0;
        }
        
        // 通用名称匹配
        if (!app.genericName.isEmpty()) {
            if (app.genericName.toLower() == searchText) {
                score += 50.0;
            }
            else if (app.genericName.toLower().contains(searchText)) {
                score += 30.0;
            }
        }
        
        // 注释匹配
        if (!app.comment.isEmpty() && app.comment.toLower().contains(searchText)) {
            score += 20.0;
        }
        
        // 关键词匹配
        for (const QString& keyword : app.keywords) {
            if (keyword.toLower().contains(searchText)) {
                score += 40.0;
                break;
            }
        }
        
        // 类别匹配
        for (const QString& category : app.categories) {
            if (category.toLower().contains(searchText)) {
                score += 10.0;
                break;
            }
        }
        
        return score;
    }
    
    // 将应用添加到搜索结果
    void addAppToResults(const AppInfo& app) {
        ResultItem item;
        
        item.name = app.name;
        item.path = app.desktopFile;
        item.type = "application/x-desktop";
        item.isDir = false;
        
        QFileInfo fileInfo(app.desktopFile);
        item.size = fileInfo.size();
        item.modifiedTime = fileInfo.lastModified();
        
        // 添加应用特有的额外数据
        item.setMetadata("exec", app.exec);
        item.setMetadata("icon", app.icon);
        item.setMetadata("comment", app.comment);
        item.setMetadata("genericName", app.genericName);
        item.setMetadata("categories", app.categories.join(';'));
        item.setMetadata("relevanceScore", app.relevanceScore);
        
        results.addItem(item);
    }
};

// 实现AppDesktopSearchEngine类的方法
AppDesktopSearchEngine::AppDesktopSearchEngine(QObject* parent) 
    : SearchEngine(parent), d(new Impl) {
    
    // 连接定时器更新进度
    connect(d->progressTimer, &QTimer::timeout, this, [this]() {
        emit progressChanged(d->progress);
    });
}

AppDesktopSearchEngine::~AppDesktopSearchEngine() {
}

QString AppDesktopSearchEngine::engineId() const {
    return "appdesktop";
}

QString AppDesktopSearchEngine::engineName() const {
    return "应用程序搜索引擎";
}

QString AppDesktopSearchEngine::engineDescription() const {
    return "搜索系统中安装的应用程序(.desktop文件)";
}

bool AppDesktopSearchEngine::supportsIndexing() const {
    // 通常应用数量不多，可以实时搜索而不需要索引
    return false;
}

bool AppDesktopSearchEngine::supportsQueryType(QueryType type) const {
    // 仅支持文件名类型的查询(应用名称、关键词等)
    return type == QueryType::Filename || type == QueryType::Extended;
}

bool AppDesktopSearchEngine::supportsMatchType(MatchType type) const {
    // 支持所有匹配类型
    return true;
}

bool AppDesktopSearchEngine::supportsFeature(const QString& feature) const {
    return feature == Features::PINYIN_SEARCH;
}

bool AppDesktopSearchEngine::prepare(const SearchQuery& query, const SearchOptions& options) {
    d->query = query;
    d->options = options;
    d->results = SearchResult();
    d->progress = 0.0;
    d->stopRequested = false;
    
    return true;
}

bool AppDesktopSearchEngine::start() {
    if (d->currentState == SearchState::Searching) {
        return false;  // 已经在搜索中
    }
    
    d->currentState = SearchState::Searching;
    d->stopRequested = false;
    emit stateChanged(d->currentState);
    
    // 开始进度更新
    d->progressTimer->start(100);
    
    // 执行搜索
    bool success = d->searchApps();
    
    // 停止进度更新
    d->progressTimer->stop();
    
    if (d->stopRequested) {
        d->currentState = SearchState::Idle;
    } else if (!success) {
        d->currentState = SearchState::Error;
    } else {
        d->currentState = SearchState::Completed;
        d->progress = 1.0;
        emit progressChanged(d->progress);
        emit searchCompleted();
    }
    
    emit stateChanged(d->currentState);
    return success;
}

bool AppDesktopSearchEngine::pause() {
    // 应用搜索通常很快，不支持暂停
    return false;
}

bool AppDesktopSearchEngine::resume() {
    // 应用搜索通常很快，不支持恢复
    return false;
}

bool AppDesktopSearchEngine::stop() {
    if (d->currentState != SearchState::Searching) {
        return false;
    }
    
    d->stopRequested = true;
    return true;
}

SearchState AppDesktopSearchEngine::state() const {
    return d->currentState;
}

double AppDesktopSearchEngine::progress() const {
    return d->progress;
}

SearchResult AppDesktopSearchEngine::currentResults() const {
    return d->results;
}

} // namespace QSearch 