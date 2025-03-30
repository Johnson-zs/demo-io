#ifndef DFM_SEARCH_ENGINE_H
#define DFM_SEARCH_ENGINE_H

#include "searchtypes.h"
#include <QObject>
#include <QTimer>
#include <QString>
#include <QIcon>
#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include <QSet>
#include <QVector>
#include <QRegularExpression>
#include <QMutex>
#include <QWaitCondition>
#include <functional>
#include <memory>
#include <vector>
#include <map>

namespace DFM {
namespace Search {

/**
 * @brief 搜索状态枚举，表示搜索引擎当前的状态
 */
enum class SearchState {
    Idle,       ///< 空闲，未开始搜索
    Preparing,
    Searching,  ///< 正在搜索
    Paused,     ///< 暂停搜索
    Completed,  ///< 搜索完成
    Cancelled,  ///< 搜索被取消
    Error       ///< 搜索出错
};

/**
 * @brief 匹配高亮信息结构体
 */
struct MatchHighlight {
    int startPos = -1;            ///< 匹配开始位置
    int length = 0;               ///< 匹配长度
    QString matchedText;          ///< 匹配的文本
};

/**
 * @brief 搜索进度信息结构体
 */
struct SearchProgress {
    int totalItems = 0;           ///< 总项目数
    int processedItems = 0;       ///< 已处理项目数
    int matchedItems = 0;         ///< 匹配项目数
    QString currentPath;          ///< 当前处理的路径
    double progressPercent = 0.0; ///< 进度百分比 (0-100)
};

/**
 * @brief 搜索结果集合类
 */
class SearchResultSet {
public:
    SearchResultSet() = default;
    
    void addResult(const std::shared_ptr<class FileResultItem>& result);
    
    int count() const { return m_results.size(); }
    bool isEmpty() const { return m_results.empty(); }
    void clear() { m_results.clear(); }
    
    const std::vector<std::shared_ptr<class FileResultItem>>& results() const { 
        return m_results; 
    }
    
private:
    std::vector<std::shared_ptr<class FileResultItem>> m_results;
};

/**
 * @brief 搜索结果接口
 */
class ISearchResult {
public:
    virtual ~ISearchResult() = default;
    
    virtual QString id() const = 0;
    virtual void setId(const QString& id) = 0;
    
    virtual QString title() const = 0;
    virtual void setTitle(const QString& title) = 0;
    
    virtual QString description() const = 0;
    virtual void setDescription(const QString& description) = 0;
    
    virtual QIcon icon() const = 0;
    virtual void setIcon(const QIcon& icon) = 0;
    
    virtual double relevance() const = 0;
    virtual void setRelevance(double relevance) = 0;
    
    virtual SearchType resultType() const = 0;
    
    virtual const QVector<MatchHighlight>& highlights() const = 0;
    virtual void addHighlight(const MatchHighlight& highlight) = 0;
    virtual void clearHighlights() = 0;
};

/**
 * @brief 文件搜索结果接口
 */
class IFileResult : public ISearchResult {
public:
    virtual ~IFileResult() = default;
    
    virtual QString path() const = 0;
    virtual void setPath(const QString& path) = 0;
    
    virtual qint64 size() const = 0;
    virtual void setSize(qint64 size) = 0;
    
    virtual QDateTime modifiedTime() const = 0;
    virtual void setModifiedTime(const QDateTime& time) = 0;
    
    virtual QDateTime createdTime() const = 0;
    virtual void setCreatedTime(const QDateTime& time) = 0;
    
    virtual bool isDir() const = 0;
    virtual void setIsDir(bool isDir) = 0;
    
    virtual QString mimeType() const = 0;
    virtual void setMimeType(const QString& mimeType) = 0;
};

/**
 * @brief 文件搜索结果实现
 */
class FileResultItem : public IFileResult {
public:
    FileResultItem() = default;
    
    // 实现ISearchResult接口
    QString id() const override;
    void setId(const QString& id) override;
    
    QString title() const override;
    void setTitle(const QString& title) override;
    
    QString description() const override;
    void setDescription(const QString& description) override;
    
    QIcon icon() const override;
    void setIcon(const QIcon& icon) override;
    
    double relevance() const override;
    void setRelevance(double relevance) override;
    
    SearchType resultType() const override { return SearchType::FileName; }
    
    const QVector<MatchHighlight>& highlights() const override;
    void addHighlight(const MatchHighlight& highlight) override;
    void clearHighlights() override;
    
    // 实现IFileResult接口
    QString path() const override;
    void setPath(const QString& path) override;
    
    qint64 size() const override;
    void setSize(qint64 size) override;
    
    QDateTime modifiedTime() const override;
    void setModifiedTime(const QDateTime& time) override;
    
    QDateTime createdTime() const override;
    void setCreatedTime(const QDateTime& time) override;
    
    bool isDir() const override;
    void setIsDir(bool isDir) override;
    
    QString mimeType() const override;
    void setMimeType(const QString& mimeType) override;
    
private:
    QString m_id;
    QString m_title;
    QString m_description;
    QIcon m_icon;
    double m_relevance = 0.0;
    
    QString m_path;
    qint64 m_size = 0;
    QDateTime m_modifiedTime;
    QDateTime m_createdTime;
    bool m_isDir = false;
    QString m_mimeType;
    QVector<MatchHighlight> m_highlights;
};

/**
 * @brief 内容搜索结果实现
 */
class ContentResultItem : public FileResultItem {
public:
    ContentResultItem() = default;
    
    SearchType resultType() const override { return SearchType::Fulltext; }
    
    // 内容特定属性
    QString content() const;
    void setContent(const QString& content);
    
    int lineNumber() const;
    void setLineNumber(int lineNumber);
    
    int columnNumber() const;
    void setColumnNumber(int columnNumber);
    
private:
    QString m_content;    // 匹配的内容
    int m_lineNumber = -1;     // 匹配的行号
    int m_columnNumber = -1;   // 匹配的列号
};

/**
 * @brief 搜索引擎接口，定义所有搜索引擎必须实现的功能
 */
class ISearchEngine : public QObject {
    Q_OBJECT

public:
    explicit ISearchEngine(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ISearchEngine() = default;

    // 搜索操作
    virtual bool prepare() = 0;
    virtual bool startSearch(const QString& query) = 0;
    virtual void pauseSearch() = 0;
    virtual void resumeSearch() = 0;
    virtual void cancelSearch() = 0;

    // 状态查询
    virtual SearchState state() const = 0;
    virtual QString lastError() const = 0;

    // 搜索能力
    virtual SearchType supportedType() const = 0;
    virtual SearchMode supportedMode() const = 0;
    virtual bool hasCapability(const QString& capability) const = 0;
    
    // 搜索引擎信息
    virtual QString name() const = 0;
    virtual QString description() const = 0;

signals:
    void stateChanged(DFM::Search::SearchState state);
    void progressChanged(const DFM::Search::SearchProgress& progress);
    void resultsReady(const DFM::Search::SearchResultSet& results);
    void searchCompleted(bool success);
    void errorOccurred(const QString& error);

protected:
    virtual void setState(SearchState state) = 0;
    virtual void setError(const QString& error) = 0;
};

/**
 * @brief 搜索引擎基类，提供共同功能的实现
 */
class BaseSearchEngine : public ISearchEngine {
    Q_OBJECT
public:
    explicit BaseSearchEngine(QObject* parent = nullptr);
    ~BaseSearchEngine() override;
    
    // ISearchEngine 接口实现
    bool pauseSearch() override;
    bool resumeSearch() override;
    bool cancelSearch() override;
    
    SearchState state() const override;
    
    void setScope(const SearchScope& scope) override;
    const SearchScope& scope() const override;
    
    void setOptions(SearchOptions options) override;
    SearchOptions options() const override;
    
    QString lastError() const override;
    
    const SearchProgress& progress() const override;
    
    // 添加结果回调
    void setResultCallback(ResultCallback callback) override;
    
protected:
    // 异步搜索帮助函数
    void startAsyncSearch(const std::function<SearchResultSet()>& searchFunc);
    
    // 更新进度信息
    void updateProgress(const SearchProgress& progress);
    
    // 设置错误信息
    void setError(const QString& error);
    
    // 更新状态
    void setState(SearchState state);
    
    // 使用回调处理单个结果
    void addResultWithCallback(const std::shared_ptr<ISearchResult>& result);
    
protected:
    SearchState m_state;
    SearchScope m_scope;
    SearchOptions m_options;
    QString m_error;
    SearchProgress m_progress;
    
    QFuture<SearchResultSet> m_searchFuture;
    QFutureWatcher<SearchResultSet>* m_searchWatcher;
    
    // 结果回调
    ResultCallback m_resultCallback;
};

/**
 * @brief 搜索引擎工厂类，用于创建和管理搜索引擎
 */
class SearchEngineFactory {
public:
    // 创建搜索引擎
    static std::shared_ptr<ISearchEngine> createEngine(SearchType type, SearchMode mode, QObject* parent = nullptr);
    
    // 注册搜索引擎
    template<typename T>
    static void registerEngine(SearchType type, SearchMode mode) {
        auto creator = [](QObject* parent) -> std::shared_ptr<ISearchEngine> {
            return std::make_shared<T>(parent);
        };
        s_registry[std::make_pair(type, mode)] = creator;
    }
    
private:
    using EngineCreator = std::function<std::shared_ptr<ISearchEngine>(QObject*)>;
    using EngineKey = std::pair<SearchType, SearchMode>;
    
    static std::map<EngineKey, EngineCreator> s_registry;
};

/**
 * @brief 搜索引擎管理器类，管理多个搜索引擎
 */
class SearchEngineManager : public QObject {
    Q_OBJECT
public:
    explicit SearchEngineManager(QObject* parent = nullptr);
    ~SearchEngineManager();
    
    // 添加搜索引擎
    void addEngine(const std::shared_ptr<ISearchEngine>& engine);
    
    // 移除搜索引擎
    void removeEngine(const std::shared_ptr<ISearchEngine>& engine);
    
    // 获取所有引擎
    QList<std::shared_ptr<ISearchEngine>> engines() const;
    
    // 按类型和模式获取引擎
    std::shared_ptr<ISearchEngine> engine(SearchType type, SearchMode mode) const;
    
    // 执行搜索
    bool search(const QString& query);
    
    // 暂停所有搜索
    void pauseAll();
    
    // 恢复所有搜索
    void resumeAll();
    
    // 取消所有搜索
    void cancelAll();
    
signals:
    void engineAdded(const std::shared_ptr<DFM::Search::ISearchEngine>& engine);
    void engineRemoved(const std::shared_ptr<DFM::Search::ISearchEngine>& engine);
    void resultsReady(SearchType type, const DFM::Search::SearchResultSet& results);
    void searchCompleted(SearchType type, bool success);
    void errorOccurred(SearchType type, const QString& error);
    
private slots:
    void onEngineStateChanged(DFM::Search::SearchState state);
    void onEngineResultsReady(const DFM::Search::SearchResultSet& results);
    void onEngineSearchCompleted(bool success);
    void onEngineErrorOccurred(const QString& error);
    
private:
    std::vector<std::shared_ptr<ISearchEngine>> m_engines;
};

} // namespace Search
} // namespace DFM

Q_DECLARE_METATYPE(DFM::Search::SearchState)
Q_DECLARE_METATYPE(DFM::Search::SearchProgress)
Q_DECLARE_METATYPE(DFM::Search::SearchResultSet)

#endif // DFM_SEARCH_ENGINE_H 
