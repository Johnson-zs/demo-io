#ifndef DFM_CONTENT_SEARCH_H
#define DFM_CONTENT_SEARCH_H

#include "searchengine.h"
#include <QDir>
#include <QFutureWatcher>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

namespace DFM {
namespace Search {

/**
 * @brief 实时文件内容搜索引擎
 * 
 * 提供基于文件系统遍历的实时文件内容搜索功能
 */
class RealtimeContentSearchEngine : public BaseSearchEngine {
    Q_OBJECT
public:
    explicit RealtimeContentSearchEngine(QObject* parent = nullptr);
    ~RealtimeContentSearchEngine() override;
    
    // 实现 ISearchEngine 接口
    bool prepare() override;
    bool startSearch(const QString& query) override;
    bool pauseSearch() override;
    bool cancelSearch() override;
    
    SearchType supportedType() const override { return SearchType::Fulltext; }
    SearchMode supportedMode() const override { return SearchMode::Realtime; }
    
    QString name() const override { return QStringLiteral("RealtimeContentSearch"); }
    QString description() const override { 
        return QStringLiteral("实时文件内容搜索，类似于 grep 命令"); 
    }
    
    bool hasCapability(const QString& capability) const override;
    
    // 设置文件类型过滤
    void setFileTypeFilters(const QStringList& filters) { m_fileTypeFilters = filters; }
    QStringList fileTypeFilters() const { return m_fileTypeFilters; }
    
    // 设置最大文件大小（字节）
    void setMaxFileSize(qint64 size) { m_maxFileSize = size; }
    qint64 maxFileSize() const { return m_maxFileSize; }
    
private:
    // 搜索实现
    SearchResultSet performSearch(const QString& query);
    
    // 递归搜索目录
    void searchDirectory(const QString& dir, const QString& query, SearchResultSet& results);
    
    // 搜索单个文件内容
    bool searchFileContent(const QString& filePath, const QString& query, SearchResultSet& results);
    
    // 检查文件类型是否应该被处理
    bool shouldProcessFile(const QFileInfo& fileInfo) const;
    
    // 从文件中读取文本内容
    QString readFileContent(const QString& filePath, qint64 maxSize = -1);
    
    // 创建内容结果项
    std::shared_ptr<ContentResultItem> createResultItem(
        const QFileInfo& fileInfo, const QString& content, 
        int lineNumber, int columnNumber, const QVector<MatchHighlight>& highlights);
    
private:
    std::atomic<bool> m_cancelRequested;
    std::atomic<bool> m_pauseRequested;
    QMutex m_pauseMutex;
    QWaitCondition m_pauseCondition;
    QStringList m_fileTypeFilters;  // 文件类型过滤 (*.txt, *.cpp 等)
    qint64 m_maxFileSize;           // 最大处理文件大小 (字节)
};

/**
 * @brief 索引文件内容搜索引擎
 * 
 * 提供基于预建索引的文件内容搜索功能
 */
class IndexedContentSearchEngine : public BaseSearchEngine {
    Q_OBJECT
public:
    explicit IndexedContentSearchEngine(QObject* parent = nullptr);
    ~IndexedContentSearchEngine() override;
    
    // 实现 ISearchEngine 接口
    bool prepare() override;
    bool startSearch(const QString& query) override;
    bool rebuildIndex();
    
    SearchType supportedType() const override { return SearchType::Fulltext; }
    SearchMode supportedMode() const override { return SearchMode::Indexed; }
    
    QString name() const override { return QStringLiteral("IndexedContentSearch"); }
    QString description() const override { 
        return QStringLiteral("索引文件内容搜索，类似于全文搜索引擎"); 
    }
    
    bool hasCapability(const QString& capability) const override;
    
    // 索引统计信息
    int indexedFileCount() const;
    QDateTime lastIndexTime() const;
    
    // 设置文件类型过滤
    void setFileTypeFilters(const QStringList& filters) { m_fileTypeFilters = filters; }
    QStringList fileTypeFilters() const { return m_fileTypeFilters; }
    
private:
    // 索引条目结构
    struct IndexEntry {
        QString path;                // 文件路径
        QString name;                // 文件名
        qint64 size;                 // 文件大小
        QDateTime modTime;           // 修改时间
        QList<QString> terms;        // 词条列表
        QHash<QString, int> termFrequency; // 词频
    };
    
    // 索引管理
    bool loadIndex();
    bool saveIndex();
    
    // 创建索引
    void buildIndex();
    
    // 索引文件
    bool indexFile(const QFileInfo& fileInfo);
    
    // 搜索索引
    SearchResultSet searchIndex(const QString& query);
    
    // 检查文件类型是否应该被处理
    bool shouldProcessFile(const QFileInfo& fileInfo) const;
    
private:
    QList<IndexEntry> m_index;       // 索引数据
    QString m_indexFilePath;         // 索引文件路径
    QDateTime m_lastIndexTime;       // 上次索引时间
    QStringList m_fileTypeFilters;   // 文件类型过滤
    std::atomic<bool> m_indexingInProgress; // 是否正在索引
};

// 注册搜索引擎
inline void registerContentSearchEngines() {
    SearchEngineFactory::registerEngine<RealtimeContentSearchEngine>(
        SearchType::Fulltext, SearchMode::Realtime);
    SearchEngineFactory::registerEngine<IndexedContentSearchEngine>(
        SearchType::Fulltext, SearchMode::Indexed);
}

} // namespace Search
} // namespace DFM

#endif // DFM_CONTENT_SEARCH_H
