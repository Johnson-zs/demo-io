#ifndef DFM_FILENAME_SEARCH_H
#define DFM_FILENAME_SEARCH_H

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
 * @brief 实时文件名搜索引擎
 * 
 * 提供基于文件系统遍历的实时文件名搜索功能
 */
class RealtimeFilenameSearchEngine : public BaseSearchEngine {
    Q_OBJECT
public:
    explicit RealtimeFilenameSearchEngine(QObject* parent = nullptr);
    ~RealtimeFilenameSearchEngine() override;
    
    // 实现 ISearchEngine 接口
    bool prepare() override;
    bool startSearch(const QString& query) override;
    bool pauseSearch() override;
    bool cancelSearch() override;
    
    SearchType supportedType() const override { return SearchType::FileName; }
    SearchMode supportedMode() const override { return SearchMode::Realtime; }
    
    QString name() const override { return QStringLiteral("RealtimeFilenameSearch"); }
    QString description() const override { 
        return QStringLiteral("实时文件名搜索，类似于 find 命令"); 
    }
    
    bool hasCapability(const QString& capability) const override;
    
private:
    // 搜索实现
    SearchResultSet performSearch(const QString& query);
    
    // 递归搜索目录
    void searchDirectory(const QString& dir, const QString& query, SearchResultSet& results);
    
    // 匹配文件名
    bool matchFilename(const QString& filename, const QString& query);
    
    // 创建文件结果项
    std::shared_ptr<FileResultItem> createResultItem(const QFileInfo& fileInfo);
    
private:
    std::atomic<bool> m_cancelRequested;
    std::atomic<bool> m_pauseRequested;
    QMutex m_pauseMutex;
    QWaitCondition m_pauseCondition;
};

/**
 * @brief 索引文件名搜索引擎
 * 
 * 提供基于预建索引的文件名搜索功能
 */
class IndexedFilenameSearchEngine : public BaseSearchEngine {
    Q_OBJECT
public:
    explicit IndexedFilenameSearchEngine(QObject* parent = nullptr);
    ~IndexedFilenameSearchEngine() override;
    
    // 实现 ISearchEngine 接口
    bool prepare() override;
    bool startSearch(const QString& query) override;
    bool rebuildIndex();
    
    SearchType supportedType() const override { return SearchType::FileName; }
    SearchMode supportedMode() const override { return SearchMode::Indexed; }
    
    QString name() const override { return QStringLiteral("IndexedFilenameSearch"); }
    QString description() const override { 
        return QStringLiteral("索引文件名搜索，类似于 locate/FSearch"); 
    }
    
    bool hasCapability(const QString& capability) const override;
    
    // 索引统计信息
    int indexedFileCount() const;
    QDateTime lastIndexTime() const;
    
private:
    // 索引管理
    bool loadIndex();
    bool saveIndex();
    
    // 创建索引
    void buildIndex();
    
    // 搜索索引
    SearchResultSet searchIndex(const QString& query);
    
private:
    // 文件索引数据结构
    // 实际实现中可能使用更高效的数据结构如 trie 树或倒排索引
    struct IndexEntry {
        QString path;
        QString name;
        qint64 size;
        QDateTime modTime;
        bool isDir;
    };
    
    QVector<IndexEntry> m_index;
    QDateTime m_lastIndexTime;
    QString m_indexFilePath;
    std::atomic<bool> m_indexingInProgress;
};

// 注册搜索引擎
inline void registerFilenameSearchEngines() {
    SearchEngineFactory::registerEngine<RealtimeFilenameSearchEngine>(
        SearchType::FileName, SearchMode::Realtime);
    SearchEngineFactory::registerEngine<IndexedFilenameSearchEngine>(
        SearchType::FileName, SearchMode::Indexed);
}

} // namespace Search
} // namespace DFM

#endif // DFM_FILENAME_SEARCH_H 
