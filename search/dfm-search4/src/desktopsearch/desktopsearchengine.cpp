#include "desktopsearchengine.h"
#include <QtConcurrent>
#include <QDirIterator>
#include <QSettings>
#include <QStandardPaths>
#include <QLocale>
#include <QDebug>

namespace DFM6 {
namespace Search {

DesktopSearchEngine::DesktopSearchEngine(QObject *parent)
    : AbstractSearchEngine(parent)
{
    // 初始化标准路径
    m_standardPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    // 添加其他可能的路径
    m_standardPaths << "/usr/share/applications";
    m_standardPaths << "/usr/local/share/applications";
    m_standardPaths << QDir::homePath() + "/.local/share/applications";
}

DesktopSearchEngine::~DesktopSearchEngine() = default;

SearchOptions DesktopSearchEngine::searchOptions() const
{
    return m_options;
}

void DesktopSearchEngine::setSearchOptions(const SearchOptions &options)
{
    // 尝试将基类选项转换为 DesktopSearchOptions
    try {
        const DesktopSearchOptions* desktopOptions = dynamic_cast<const DesktopSearchOptions*>(&options);
        if (desktopOptions) {
            m_options = *desktopOptions;
        } else {
            m_options = DesktopSearchOptions(options);
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to convert search options:" << e.what();
        m_options = DesktopSearchOptions(options);
    }
    
    // 如果用户设置了搜索路径，则覆盖默认路径
    if (!m_options.searchPath().isEmpty()) {
        if (m_options.isRecursive()) {
            m_standardPaths.clear();
            m_standardPaths << m_options.searchPath();
        } else {
            // 非递归模式下，仅在指定路径中搜索
            m_standardPaths = QStringList{m_options.searchPath()};
        }
    }
}

SearchStatus DesktopSearchEngine::status() const
{
    return m_status.load();
}

QFuture<QList<SearchResult>> DesktopSearchEngine::search(const SearchQuery &query)
{
    if (m_status.load() == SearchStatus::Searching) {
        return QtConcurrent::run([]() { return QList<SearchResult>(); });
    }
    
    m_cancelled.store(false);
    setStatus(SearchStatus::Searching);
    emit searchStarted();
    
    return QtConcurrent::run([this, query]() {
        QList<SearchResult> results = searchSync(query);
        
        if (!m_cancelled.load()) {
            setStatus(SearchStatus::Finished);
            emit searchFinished(results);
        }
        
        return results;
    });
}

QFuture<void> DesktopSearchEngine::searchWithCallback(const SearchQuery &query, 
                                                    SearchEngine::ResultCallback callback)
{
    if (m_status.load() == SearchStatus::Searching) {
        return QtConcurrent::run([]() {});
    }
    
    m_cancelled.store(false);
    setStatus(SearchStatus::Searching);
    emit searchStarted();
    
    return QtConcurrent::run([this, query, callback]() {
        QList<SearchResult> results;
        int fileCount = 0;
        int matchCount = 0;
        
        // 遍历所有标准路径
        for (const QString &path : m_standardPaths) {
            if (m_cancelled.load()) break;
            
            QDirIterator it(path, {"*.desktop"}, QDir::Files, 
                m_options.isRecursive() ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
                
            while (it.hasNext()) {
                if (m_cancelled.load()) break;
                
                // 处理暂停
                processPauseIfNeeded();
                
                QString filePath = it.next();
                fileCount++;
                
                // 解析并处理desktop文件
                SearchResult result = parseDesktopFile(filePath, query);
                if (!result.path().isEmpty()) {
                    matchCount++;
                    
                    if (callback) {
                        callback(result);
                    }
                    
                    emit resultFound(result);
                    results.append(result);
                    
                    // 检查是否达到最大结果数
                    if (m_options.maxResults() > 0 && matchCount >= m_options.maxResults()) {
                        break;
                    }
                }
                
                // 每处理10个文件报告一次进度
                if (fileCount % 10 == 0) {
                    emit progressChanged(fileCount, -1); // -1表示总数未知
                    emit searchStatusUpdate(fileCount, matchCount, 0, 0);
                }
            }
            
            if (m_options.maxResults() > 0 && matchCount >= m_options.maxResults()) {
                break;
            }
        }
        
        // 如果需要排序结果
        if (m_options.sortResults() && !m_cancelled.load()) {
            std::sort(results.begin(), results.end(), [](const SearchResult &a, const SearchResult &b) {
                return a.score() > b.score(); // 按分数降序排列
            });
        }
        
        if (!m_cancelled.load()) {
            setStatus(SearchStatus::Finished);
            emit searchFinished(results);
        }
    });
}

QList<SearchResult> DesktopSearchEngine::searchSync(const SearchQuery &query)
{
    QList<SearchResult> results;
    
    if (m_cancelled.load()) {
        return results;
    }
    
    try {
        int fileCount = 0;
        int matchCount = 0;
        
        // 遍历所有标准路径
        for (const QString &path : m_standardPaths) {
            if (m_cancelled.load()) break;
            
            QDirIterator it(path, {"*.desktop"}, QDir::Files, 
                m_options.isRecursive() ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
                
            while (it.hasNext()) {
                if (m_cancelled.load()) break;
                
                // 处理暂停
                processPauseIfNeeded();
                
                QString filePath = it.next();
                fileCount++;
                
                // 解析并处理desktop文件
                SearchResult result = parseDesktopFile(filePath, query);
                if (!result.path().isEmpty()) {
                    matchCount++;
                    results.append(result);
                    
                    // 检查是否达到最大结果数
                    if (m_options.maxResults() > 0 && matchCount >= m_options.maxResults()) {
                        break;
                    }
                }
                
                // 每处理10个文件报告一次进度
                if (fileCount % 10 == 0) {
                    emit progressChanged(fileCount, -1);
                }
            }
            
            if (m_options.maxResults() > 0 && matchCount >= m_options.maxResults()) {
                break;
            }
        }
        
        // 如果需要排序结果
        if (m_options.sortResults() && !m_cancelled.load()) {
            std::sort(results.begin(), results.end(), [](const SearchResult &a, const SearchResult &b) {
                return a.score() > b.score(); // 按分数降序排列
            });
        }
        
        return results;
    }
    catch (const std::exception& e) {
        qWarning() << "Desktop search error:" << e.what();
        reportError(QString("Search error: %1").arg(e.what()));
        return QList<SearchResult>();
    }
}

void DesktopSearchEngine::pause()
{
    if (m_status.load() == SearchStatus::Searching) {
        setStatus(SearchStatus::Paused);
    }
}

void DesktopSearchEngine::resume()
{
    if (m_status.load() == SearchStatus::Paused) {
        m_mutex.lock();
        setStatus(SearchStatus::Searching);
        m_pauseCondition.wakeAll();
        m_mutex.unlock();
    }
}

void DesktopSearchEngine::cancel()
{
    m_cancelled.store(true);
    
    if (m_status.load() == SearchStatus::Paused) {
        m_mutex.lock();
        m_pauseCondition.wakeAll();
        m_mutex.unlock();
    }
    
    if (m_status.load() != SearchStatus::Ready && 
        m_status.load() != SearchStatus::Finished) {
        setStatus(SearchStatus::Cancelled);
        emit searchCancelled();
    }
}

void DesktopSearchEngine::clearCache()
{
    // 桌面搜索不使用缓存
}

void DesktopSearchEngine::processPauseIfNeeded()
{
    m_mutex.lock();
    if (m_status.load() == SearchStatus::Paused) {
        m_pauseCondition.wait(&m_mutex);
    }
    m_mutex.unlock();
}

SearchResult DesktopSearchEngine::parseDesktopFile(const QString &filePath, const SearchQuery &query)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        return SearchResult();
    }
    
    // 使用QSettings解析desktop文件
    QSettings desktopFile(filePath, QSettings::IniFormat);
    desktopFile.setIniCodec("UTF-8");
    
    // 检查是否是有效的Desktop Entry
    if (!desktopFile.childGroups().contains("Desktop Entry")) {
        return SearchResult();
    }
    
    desktopFile.beginGroup("Desktop Entry");
    
    // 获取关键字段
    QMap<QString, QString> desktopData;
    desktopData["Type"] = desktopFile.value("Type").toString();
    
    // 检查Type是否为Application
    if (desktopData["Type"] != "Application") {
        desktopFile.endGroup();
        return SearchResult();
    }
    
    // 检查NoDisplay字段
    if (m_options.onlyShowVisible() && desktopFile.value("NoDisplay", false).toBool()) {
        desktopFile.endGroup();
        return SearchResult();
    }
    
    // 如果有类别过滤
    if (!m_options.categories().isEmpty()) {
        QStringList fileCategories = desktopFile.value("Categories").toString().split(';', Qt::SkipEmptyParts);
        bool matchCategory = false;
        
        for (const QString &category : m_options.categories()) {
            if (fileCategories.contains(category)) {
                matchCategory = true;
                break;
            }
        }
        
        if (!matchCategory) {
            desktopFile.endGroup();
            return SearchResult();
        }
    }
    
    // 获取本地化名称，如果没有则使用默认名称
    QString locale = QLocale::system().name();
    QString nameKey = QString("Name[%1]").arg(locale);
    desktopData["Name"] = desktopFile.contains(nameKey) ? 
                          desktopFile.value(nameKey).toString() : 
                          desktopFile.value("Name").toString();
    
    // 获取本地化描述
    QString commentKey = QString("Comment[%1]").arg(locale);
    desktopData["Comment"] = desktopFile.contains(commentKey) ? 
                             desktopFile.value(commentKey).toString() : 
                             desktopFile.value("Comment").toString();
    
    // 获取关键词
    QString keywordsKey = QString("Keywords[%1]").arg(locale);
    desktopData["Keywords"] = desktopFile.contains(keywordsKey) ? 
                              desktopFile.value(keywordsKey).toString() : 
                              desktopFile.value("Keywords").toString();
    
    // 获取执行命令
    desktopData["Exec"] = desktopFile.value("Exec").toString();
    
    // 获取图标
    desktopData["Icon"] = desktopFile.value("Icon").toString();
    
    // 获取类别
    desktopData["Categories"] = desktopFile.value("Categories").toString();
    
    desktopFile.endGroup();
    
    // 检查是否匹配查询
    if (!matchesQuery(fileInfo, desktopData, query)) {
        return SearchResult();
    }
    
    // 创建搜索结果
    SearchResult result(filePath);
    result.setSize(fileInfo.size());
    result.setModifiedTime(fileInfo.lastModified());
    result.setScore(calculateScore(desktopData, query));
    
    // 设置自定义属性
    result.setCustomAttribute("applicationName", desktopData["Name"]);
    result.setCustomAttribute("description", desktopData["Comment"]);
    result.setCustomAttribute("icon", desktopData["Icon"]);
    result.setCustomAttribute("exec", desktopData["Exec"]);
    result.setCustomAttribute("categories", desktopData["Categories"]);
    
    return result;
}

bool DesktopSearchEngine::matchesQuery(const QFileInfo &fileInfo, const QMap<QString, QString> &desktopData, const SearchQuery &query)
{
    // 如果查询为空，返回所有桌面文件
    if (query.keyword().isEmpty()) {
        return true;
    }
    
    QString keyword = query.keyword().toLower();
    
    // 检查名称是否匹配
    if (m_options.searchName() && desktopData["Name"].toLower().contains(keyword)) {
        return true;
    }
    
    // 检查描述是否匹配
    if (m_options.searchDescription() && desktopData["Comment"].toLower().contains(keyword)) {
        return true;
    }
    
    // 检查关键词是否匹配
    if (m_options.searchKeywords() && desktopData["Keywords"].toLower().contains(keyword)) {
        return true;
    }
    
    // 检查文件名是否匹配
    if (fileInfo.fileName().toLower().contains(keyword)) {
        return true;
    }
    
    return false;
}

float DesktopSearchEngine::calculateScore(const QMap<QString, QString> &desktopData, const SearchQuery &query)
{
    if (query.keyword().isEmpty()) {
        return 0.5f; // 默认分数
    }
    
    float score = 0.0f;
    QString keyword = query.keyword().toLower();
    
    // 名称匹配得分最高
    if (desktopData["Name"].toLower().contains(keyword)) {
        score += 0.5f;
        // 如果是精确匹配，分数更高
        if (desktopData["Name"].toLower() == keyword) {
            score += 0.3f;
        }
    }
    
    // 关键词匹配得分次之
    if (desktopData["Keywords"].toLower().contains(keyword)) {
        score += 0.3f;
    }
    
    // 描述匹配得分最低
    if (desktopData["Comment"].toLower().contains(keyword)) {
        score += 0.2f;
    }
    
    // 确保分数在0-1之间
    return qMin(1.0f, score);
}

}  // namespace Search
}  // namespace DFM6 