#ifndef LUCENESEARCHENGINE_H
#define LUCENESEARCHENGINE_H

#include "searchengine.h"
#include <lucene++/LuceneHeaders.h>
#include <QFileInfo>
#include <QStringList>
#include "filedata.h"
#include <memory>

using namespace Lucene;

// 前向声明
class QueryBuilder;
class SearchCache;
class IndexManager;

/**
 * @brief Lucene 搜索引擎实现
 *
 * 使用 Lucene++ 库实现高性能文件搜索
 */
class LuceneSearchEngine : public ISearchEngine
{
public:
    /**
     * @brief 构造函数
     */
    LuceneSearchEngine();

    /**
     * @brief 析构函数
     */
    virtual ~LuceneSearchEngine();

    // 实现基类方法
    void updateSearchPath(const QString &path) override;
    void cancelSearch() override;
    QVector<FileData> searchFiles(const QString &keyword, bool caseSensitive = false, bool fuzzySearch = false) const override;
    QVector<FileData> searchFilesBatch(const QString &keyword, int offset, int limit, bool caseSensitive = false, bool fuzzySearch = false) const override;
    QVector<FileData> getAllFiles(int limit = -1) const override;
    void clearCache() override;
    int getSearchResultCount(const QString &keyword, bool caseSensitive = false, bool fuzzySearch = false) const override;
    void warmupIndex() const;

private:
    // 搜索类型枚举，用于确定使用哪种查询策略
    enum class SearchType {
        Simple,   // 简单搜索
        Boolean,   // 布尔搜索（多关键词）
        Fuzzy,   // 模糊搜索
        Wildcard   // 通配符搜索
    };

    // 辅助方法
    SearchType determineSearchType(const QString &keyword, bool fuzzySearch) const;
    Lucene::QueryPtr buildSearchQuery(const QString &keyword, bool caseSensitive, bool fuzzySearch) const;

    // 执行 Lucene 搜索，核心方法
    QStringList performLuceneSearch(const QString &path, const QString &keyword, bool nrt, bool caseSensitive, bool fuzzySearch) const;

    // 文件路径转换为文件数据
    QVector<FileData> convertToFileData(const QStringList &paths) const;

    // 路径处理
    QString getIndexDirectory() const;
    QString getHomeDirectory() const;

    // 成员变量
    QString m_currentPath;   // 当前搜索路径
    mutable bool m_searchCancelled;   // 搜索取消标志

    // 缓存相关
    mutable QStringList m_cachedAllFiles;   // 缓存的所有文件列表
    mutable QString m_lastKeyword;   // 上次搜索的关键词
    mutable QStringList m_lastSearchResults;   // 上次搜索的结果
    mutable bool m_lastCaseSensitive;   // 上次搜索是否区分大小写
    mutable bool m_lastFuzzySearch;   // 上次搜索是否模糊搜索

    // 组合组件
    std::unique_ptr<QueryBuilder> m_queryBuilder;   // 查询构建器
    std::unique_ptr<SearchCache> m_searchCache;   // 搜索缓存
    std::unique_ptr<IndexManager> m_indexManager;   // 索引管理器
};

/**
 * @brief 查询构建器类
 *
 * 负责构建各种类型的 Lucene 查询
 */
class QueryBuilder
{
public:
    QueryBuilder();

    /**
     * @brief 构建类型搜索查询
     */
    Lucene::QueryPtr buildTypeQuery(const QString &typeList) const;

    /**
     * @brief 构建拼音搜索查询
     */
    Lucene::QueryPtr buildPinyinQuery(const QString &pinyinList) const;

    /**
     * @brief 构建模糊搜索查询
     */
    Lucene::QueryPtr buildFuzzyQuery(const QString &keyword, bool caseSensitive) const;

    /**
     * @brief 构建布尔搜索查询
     */
    Lucene::QueryPtr buildBooleanQuery(const QString &keyword, bool caseSensitive) const;

    /**
     * @brief 构建通配符搜索查询
     */
    Lucene::QueryPtr buildWildcardQuery(const QString &keyword, bool caseSensitive) const;

    /**
     * @brief 构建简单搜索查询
     */
    Lucene::QueryPtr buildSimpleQuery(const QString &keyword, bool caseSensitive) const;

    /**
     * @brief 处理字符串大小写
     */
    Lucene::String processString(const QString &str, bool caseSensitive) const;
};

/**
 * @brief 搜索缓存类
 *
 * 管理搜索结果缓存
 */
class SearchCache
{
public:
    SearchCache();

    /**
     * @brief 获取缓存结果
     */
    QStringList getCachedResults(const QString &keyword, bool caseSensitive, bool fuzzySearch) const;

    /**
     * @brief 存储结果到缓存
     */
    void storeResults(const QString &keyword, const QStringList &results, bool caseSensitive, bool fuzzySearch);

    /**
     * @brief 清除缓存
     */
    void clearCache();

    /**
     * @brief 检查是否命中缓存
     */
    bool isCacheHit(const QString &keyword, bool caseSensitive, bool fuzzySearch) const;

private:
    QString m_lastKeyword;
    QStringList m_lastResults;
    bool m_lastCaseSensitive;
    bool m_lastFuzzySearch;
};

/**
 * @brief 索引管理器类
 *
 * 管理 Lucene 索引目录和读取器
 */
class IndexManager
{
public:
    IndexManager();

    /**
     * @brief 获取索引目录
     */
    FSDirectoryPtr getIndexDirectory(const QString &indexPath) const;

    /**
     * @brief 获取索引读取器
     */
    IndexReaderPtr getIndexReader(FSDirectoryPtr directory) const;

    /**
     * @brief 获取搜索器
     */
    SearcherPtr getSearcher(IndexReaderPtr reader) const;

    /**
     * @brief 预热索引
     */
    void warmupIndex(const QString &indexPath) const;

private:
    mutable FSDirectoryPtr m_cachedDirectory;
    mutable IndexReaderPtr m_cachedReader;
    mutable SearcherPtr m_cachedSearcher;
    mutable QString m_cachedIndexPath;
};

#endif   // LUCENESEARCHENGINE_H
