#include "dfm-search/appdataprovider.h"
#include "dfm-search/searchresult.h"
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QDebug>

namespace DFMSearch {

// 私有实现类
class AppDataProviderPrivate {
public:
    QStringList appDirectories;
};

AppDataProvider::AppDataProvider(QObject* parent)
    : SearchProvider(parent)
    , d_ptr(std::make_unique<AppDataProviderPrivate>())
{
    // 设置默认应用程序目录
    d_ptr->appDirectories = {
        "/usr/share/applications",
        QDir::homePath() + "/.local/share/applications"
    };
}

AppDataProvider::~AppDataProvider() = default;

QString AppDataProvider::name() const
{
    return QStringLiteral("应用程序搜索");
}

QString AppDataProvider::description() const
{
    return QStringLiteral("搜索系统中已安装的应用程序");
}

SearchType AppDataProvider::searchType() const
{
    return SearchType::Application;
}

SearchMechanism AppDataProvider::mechanism() const
{
    return SearchMechanism::RealTime;
}

void AppDataProvider::setAppDirectories(const QStringList& directories)
{
    d_ptr->appDirectories = directories;
}

QStringList AppDataProvider::appDirectories() const
{
    return d_ptr->appDirectories;
}

bool AppDataProvider::doSearch()
{
    const QString keyword = query().keyword();
    if (keyword.isEmpty()) {
        reportError(QStringLiteral("搜索关键词不能为空"));
        return false;
    }
    
    const bool caseSensitive = query().hasFlag(SearchFlag::CaseSensitive);
    const bool regexSupport = query().hasFlag(SearchFlag::RegexSupport);
    const bool fuzzyMatch = query().hasFlag(SearchFlag::FuzzyMatch);
    const int limit = query().limit();
    
    int resultCount = 0;
    int progress = 0;
    int totalFiles = 0;
    
    // 计算总文件数用于进度显示
    for (const auto& dir : d_ptr->appDirectories) {
        QDir directory(dir);
        QStringList desktopFiles = directory.entryList({"*.desktop"}, QDir::Files);
        totalFiles += desktopFiles.count();
    }
    
    if (totalFiles == 0) {
        reportError(QStringLiteral("未找到应用程序目录或目录为空"));
        return false;
    }
    
    int processedFiles = 0;
    
    // 遍历应用目录
    for (const auto& dir : d_ptr->appDirectories) {
        QDir directory(dir);
        QStringList desktopFiles = directory.entryList({"*.desktop"}, QDir::Files);
        
        for (const auto& file : desktopFiles) {
            QString desktopFilePath = dir + "/" + file;
            QSettings desktopFile(desktopFilePath, QSettings::IniFormat);
            desktopFile.beginGroup("Desktop Entry");
            
            const QString name = desktopFile.value("Name").toString();
            const QString genericName = desktopFile.value("GenericName").toString();
            const QString comment = desktopFile.value("Comment").toString();
            const QString categories = desktopFile.value("Categories").toString();
            const QString exec = desktopFile.value("Exec").toString();
            const QString icon = desktopFile.value("Icon").toString();
            
            // 组合所有可能匹配的文本
            QString searchableText = name + " " + genericName + " " + comment + " " + categories;
            
            bool isMatch = false;
            
            if (regexSupport) {
                // 正则表达式匹配
                QRegularExpression regex(keyword, 
                    caseSensitive ? QRegularExpression::NoPatternOption 
                                  : QRegularExpression::CaseInsensitiveOption);
                isMatch = regex.match(searchableText).hasMatch();
            } else if (fuzzyMatch) {
                // 模糊匹配
                if (caseSensitive) {
                    isMatch = searchableText.contains(keyword);
                } else {
                    isMatch = searchableText.toLower().contains(keyword.toLower());
                }
            } else {
                // 精确匹配（名称或GenericName）
                if (caseSensitive) {
                    isMatch = name == keyword || genericName == keyword;
                } else {
                    isMatch = name.toLower() == keyword.toLower() || 
                              genericName.toLower() == keyword.toLower();
                }
            }
            
            if (isMatch) {
                AppSearchResult result;
                result.setTitle(name);
                result.setPath(desktopFilePath);
                result.setDescription(comment);
                result.setIconPath(icon);
                result.setExecuteCommand(exec.split(" ").first()); // 移除参数
                
                addResult(result);
                
                ++resultCount;
                if (limit > 0 && resultCount >= limit) {
                    setProgress(100);
                    reportCompleted();
                    return true;
                }
            }
            
            ++processedFiles;
            int newProgress = processedFiles * 100 / totalFiles;
            if (newProgress != progress) {
                progress = newProgress;
                setProgress(progress);
            }
        }
    }
    
    setProgress(100);
    reportCompleted();
    return true;
}

} // namespace DFMSearch 