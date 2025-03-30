#ifndef DFM_SEARCH_ENGINE_H
#define DFM_SEARCH_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <optional>
#include <variant>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QDateTime>

namespace DFM {
namespace Search {

// 前向声明
class SearchQuery;
class SearchResult;
class SearchOptions;
class SearchEngine;

/**
 * @brief 搜索类型枚举
 */
enum class SearchType {
    Filename,       ///< 文件名搜索
    FileContent,    ///< 文件内容搜索
    Application,    ///< 应用程序搜索
    ImageContent,   ///< 图片内容搜索（OCR）
    Custom          ///< 自定义搜索类型
};

/**
 * @brief 搜索机制枚举
 */
enum class SearchMechanism {
    Indexed,    ///< 基于索引的搜索
    Realtime    ///< 实时搜索（直接遍历）
};

/**
 * @brief 搜索状态枚举
 */
enum class SearchStatus {
    Ready,      ///< 就绪，未开始
    Running,    ///< 搜索中
    Paused,     ///< 已暂停
    Completed,  ///< 已完成
    Cancelled,  ///< 已取消
    Error       ///< 错误状态
};

/**
 * @brief 搜索选项类
 */
class SearchOptions {
public:
    SearchOptions();
    ~SearchOptions();
    
    // Getter方法
    bool caseSensitive() const;
    bool useRegex() const;
    bool fuzzyMatch() const;
    bool pinyinMatch() const;
    int maxResults() const;
    int pageSize() const;
    std::chrono::milliseconds timeout() const;
    const QVariantMap& customOptions() const;
    QVariantMap& customOptions();
    const QStringList& searchPaths() const;
    QStringList& searchPaths();
    const QStringList& excludePaths() const;
    QStringList& excludePaths();
    const QStringList& fileFilters() const;
    QStringList& fileFilters();
    
    // Setter方法
    void setCaseSensitive(bool sensitive);
    void setUseRegex(bool useRegex);
    void setFuzzyMatch(bool fuzzy);
    void setPinyinMatch(bool pinyin);
    void setMaxResults(int maxResults);
    void setPageSize(int pageSize);
    void setTimeout(std::chrono::milliseconds timeout);
    void setCustomOptions(const QVariantMap& options);
    void setSearchPaths(const QStringList& paths);
    void setExcludePaths(const QStringList& paths);
    void setFileFilters(const QStringList& filters);
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 搜索结果项基类
 */
class SearchResultItem {
public:
    SearchResultItem();
    virtual ~SearchResultItem();
    
    // 访问器方法
    QString displayName() const;
    void setDisplayName(const QString& name);
    
    QUrl uri() const;
    void setUri(const QUrl& uri);
    
    QDateTime lastModified() const;
    void setLastModified(const QDateTime& dateTime);
    
    double relevanceScore() const;
    void setRelevanceScore(double score);
    
    QVariantMap metadata() const;
    void setMetadata(const QVariantMap& meta);
    void addMetadata(const QString& key, const QVariant& value);

    virtual SearchType resultType() const = 0;
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 文件搜索结果
 */
class FileResultItem : public SearchResultItem {
public:
    FileResultItem();
    ~FileResultItem() override;
    
    QString filePath() const;
    void setFilePath(const QString& path);
    
    qint64 fileSize() const;
    void setFileSize(qint64 size);
    
    QString fileType() const;
    void setFileType(const QString& type);
    
    bool isDirectory() const;
    void setIsDirectory(bool isDir);

    SearchType resultType() const override { return SearchType::Filename; }
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 文件内容搜索结果
 */
class ContentResultItem : public FileResultItem {
public:
    ContentResultItem();
    ~ContentResultItem() override;
    
    QString matchedContent() const;
    void setMatchedContent(const QString& content);
    
    int lineNumber() const;
    void setLineNumber(int line);
    
    int charPosition() const;
    void setCharPosition(int pos);
    
    SearchType resultType() const override { return SearchType::FileContent; }
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 应用程序搜索结果
 */
class AppResultItem : public SearchResultItem {
public:
    AppResultItem();
    ~AppResultItem() override;
    
    QString appId() const;
    void setAppId(const QString& id);
    
    QString desktopFilePath() const;
    void setDesktopFilePath(const QString& path);
    
    QString iconName() const;
    void setIconName(const QString& name);
    
    QString execCommand() const;
    void setExecCommand(const QString& cmd);
    
    SearchType resultType() const override { return SearchType::Application; }
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 图片内容搜索结果（OCR）
 */
class ImageResultItem : public FileResultItem {
public:
    ImageResultItem();
    ~ImageResultItem() override;
    
    QString ocrText() const;
    void setOcrText(const QString& text);
    
    QRect textRegion() const;
    void setTextRegion(const QRect& region);
    
    SearchType resultType() const override { return SearchType::ImageContent; }
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 搜索结果容器类
 */
class SearchResult {
public:
    using ResultItemPtr = std::shared_ptr<SearchResultItem>;
    
    SearchResult();
    ~SearchResult();
    
    // 查询方法
    const std::vector<ResultItemPtr>& items() const;
    bool hasMoreResults() const;
    int totalResultCount() const;
    int currentPage() const;
    double searchTime() const;
    
    // 设置方法
    void setHasMoreResults(bool hasMore);
    void setTotalResultCount(int count);
    void setCurrentPage(int page);
    void setSearchTime(double time);
    
    // 添加结果项
    void addItem(ResultItemPtr item);
    
    // 清空结果
    void clear();
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 搜索查询类
 */
class SearchQuery {
public:
    SearchQuery();
    ~SearchQuery();
    
    // 查询方法
    QString queryString() const;
    SearchType type() const;
    SearchMechanism mechanism() const; 
    const SearchOptions& options() const;
    SearchOptions& options();
    
    // 设置方法
    void setQueryString(const QString& query);
    void setType(SearchType type);
    void setMechanism(SearchMechanism mechanism);
    void setOptions(const SearchOptions& options);
    
    // 创建不同类型的查询的工厂方法
    static SearchQuery createFilenameQuery(const QString& query);
    static SearchQuery createContentQuery(const QString& query);
    static SearchQuery createAppQuery(const QString& query);
    
private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @brief 搜索引擎接口基类
 * 
 * 注意：使用PIMPL模式避免ABI兼容性问题
 */
class SearchEngine : public QObject {
    Q_OBJECT
    
public:
    // 使用PIMPL模式以保持ABI兼容性
    class Impl;
    
    SearchEngine();
    virtual ~SearchEngine();
    
    // 搜索控制
    bool search(const SearchQuery& query);
    bool pause();
    bool resume();
    bool cancel();
    
    // 状态查询
    SearchStatus status() const;
    int progressPercentage() const;
    
    // 结果获取
    SearchResult getResults(int pageIndex = 0, int pageSize = -1) const;
    
    // 设置回调
    using ResultCallback = std::function<void(const SearchResult&)>;
    using ProgressCallback = std::function<void(int percentage)>;
    using StatusCallback = std::function<void(SearchStatus)>;
    
    void setResultCallback(ResultCallback callback);
    void setProgressCallback(ProgressCallback callback);
    void setStatusCallback(StatusCallback callback);
    
signals:
    // Qt信号，提供与回调函数相同的功能
    void resultsReady(const DFM::Search::SearchResult& results);
    void progressChanged(int percentage);
    void statusChanged(DFM::Search::SearchStatus status);
    void searchCompleted();
    void searchCancelled();
    void searchError(const QString& errorMessage);
    
private:
    std::unique_ptr<Impl> d; // 私有实现
};

/**
 * @brief 搜索引擎管理器
 * 
 * 负责创建和管理不同类型的搜索引擎实例
 */
class SearchEngineManager {
public:
    static SearchEngineManager& instance();
    
    // 创建搜索引擎
    std::shared_ptr<SearchEngine> createEngine(SearchType type, SearchMechanism mechanism);
    
    // 注册自定义搜索引擎工厂
    using EngineCreator = std::function<std::shared_ptr<SearchEngine>()>;
    void registerEngineCreator(SearchType type, SearchMechanism mechanism, EngineCreator creator);
    
private:
    SearchEngineManager() = default;
    ~SearchEngineManager() = default;
    
    // 禁止拷贝
    SearchEngineManager(const SearchEngineManager&) = delete;
    SearchEngineManager& operator=(const SearchEngineManager&) = delete;
};

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_ENGINE_H 