#include "contentsearchengine.h"
#include <QtConcurrent>
#include <QDebug>
#include <QFile>
#include <QTextStream>

// 前向声明，实际项目中需要包含真实的头文件
class ContentSearcher;
struct SearchResult {
    QString path;
    QString content;
};

namespace DFM6 {
namespace Search {

ContentSearchEngine::ContentSearchEngine(QObject *parent)
    : AbstractSearchEngine(parent),
      m_searcher(nullptr)
{
    // 实际项目中这里应初始化 m_searcher
    // m_searcher = std::make_unique<ContentSearcher>();
}

ContentSearchEngine::~ContentSearchEngine() = default;

SearchOptions ContentSearchEngine::searchOptions() const
{
    return m_options;
}

void ContentSearchEngine::setSearchOptions(const SearchOptions &options)
{
    // 尝试将基类选项转换为 ContentSearchOptions
    try {
        const ContentSearchOptions* contentOptions = dynamic_cast<const ContentSearchOptions*>(&options);
        if (contentOptions) {
            m_options = *contentOptions;
        } else {
            m_options = ContentSearchOptions(options);
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to convert search options:" << e.what();
        m_options = ContentSearchOptions(options);
    }
    
    // 配置底层搜索器
    if (m_searcher) {
        // TODO: 配置 m_searcher 的选项
    }
}

SearchStatus ContentSearchEngine::status() const
{
    return m_status.load();
}

QFuture<QList<SearchResult>> ContentSearchEngine::search(const SearchQuery &query)
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

QFuture<void> ContentSearchEngine::searchWithCallback(const SearchQuery &query, 
                                                    SearchEngine::ResultCallback callback)
{
    if (m_status.load() == SearchStatus::Searching) {
        return QtConcurrent::run([]() {});
    }
    
    m_cancelled.store(false);
    setStatus(SearchStatus::Searching);
    emit searchStarted();
    
    return QtConcurrent::run([this, query, callback]() {
        QList<SearchResult> results = searchSync(query);
        
        for (const auto &result : results) {
            if (m_cancelled.load()) {
                break;
            }
            
            // 暂停处理
            m_mutex.lock();
            if (m_status.load() == SearchStatus::Paused) {
                m_pauseCondition.wait(&m_mutex);
            }
            m_mutex.unlock();
            
            if (callback) {
                callback(result);
            }
            
            emit resultFound(result);
        }
        
        if (!m_cancelled.load()) {
            setStatus(SearchStatus::Finished);
            emit searchFinished(results);
        }
    });
}

QList<SearchResult> ContentSearchEngine::searchSync(const SearchQuery &query)
{
    // 保存当前查询以便供 convertResults 使用
    m_currentQuery = query;
    
    // 注：这里只是演示，实际项目中应使用 ContentSearcher 实现
    QList<SearchResult> results;
    
    if (m_cancelled.load()) {
        return results;
    }
    
    try {
        QStringList keywords = processQuery(query);
        
        // 假设的搜索结果
        QList<::SearchResult> fakeResults;
        
        // 在实际实现中，这里应调用 m_searcher 进行搜索
        // if (m_searcher) {
        //     fakeResults = m_searcher->searchContent(keywords, m_options.searchPath());
        // }
        
        // 模拟一些搜索结果
        ::SearchResult fakeResult;
        fakeResult.path = m_options.searchPath() + "/document.txt";
        fakeResult.content = "This is a sample content with the keywords";
        fakeResults.append(fakeResult);
        
        return convertResults(fakeResults);
    } 
    catch (const std::exception& e) {
        qWarning() << "Content search error:" << e.what();
        reportError(QString("Search error: %1").arg(e.what()));
        return QList<SearchResult>();
    }
}

void ContentSearchEngine::pause()
{
    if (m_status.load() == SearchStatus::Searching) {
        setStatus(SearchStatus::Paused);
    }
}

void ContentSearchEngine::resume()
{
    if (m_status.load() == SearchStatus::Paused) {
        m_mutex.lock();
        setStatus(SearchStatus::Searching);
        m_pauseCondition.wakeAll();
        m_mutex.unlock();
    }
}

void ContentSearchEngine::cancel()
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

void ContentSearchEngine::clearCache()
{
    // 在实际实现中，这里应清除 m_searcher 的缓存
    // if (m_searcher) {
    //     m_searcher->clearCache();
    // }
}

QStringList ContentSearchEngine::processQuery(const SearchQuery &query)
{
    // 将 SearchQuery 转换为关键词列表
    QStringList keywords;
    
    switch (query.type()) {
    case SearchQuery::Type::Simple:
        keywords << query.keyword();
        break;
        
    case SearchQuery::Type::Boolean:
        // 对于布尔查询，添加所有子查询的关键词
        if (query.subQueries().isEmpty()) {
            keywords << query.keyword();
        } else {
            for (const auto &subQuery : query.subQueries()) {
                keywords << subQuery.keyword();
            }
        }
        break;
        
    case SearchQuery::Type::Wildcard:
    case SearchQuery::Type::Fuzzy:
    case SearchQuery::Type::Regex:
        // 原样传递特殊查询关键词
        keywords << query.keyword();
        break;
    }
    
    return keywords;
}

QList<DFM6::Search::SearchResult> ContentSearchEngine::convertResults(const QList<::SearchResult> &results)
{
    QList<DFM6::Search::SearchResult> convertedResults;
    
    for (const auto &result : results) {
        DFM6::Search::SearchResult searchResult(result.path);
        
        // 处理高亮内容
        if (!result.content.isEmpty()) {
            // 优先使用已经高亮的内容
            searchResult.setHighlightedContent(result.content);
        } else {
            // 如果底层引擎没有提供高亮内容，可以尝试自己实现
            // 例如，读取文件的一小部分并高亮关键词
            QFile file(result.path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString content = in.read(m_options.maxPreviewLength());
                file.close();
                
                // 使用工具函数高亮关键词
                QStringList keywords = processQuery(m_currentQuery);
                QString highlighted = DFM6::Search::Utils::SearchUtility::highlightKeywords(
                    content, keywords, "<b>", "</b>");
                searchResult.setHighlightedContent(highlighted);
            }
        }
        
        // 设置文件信息
        QFileInfo fileInfo(result.path);
        searchResult.setSize(fileInfo.size());
        searchResult.setModifiedTime(fileInfo.lastModified());
        searchResult.setIsDirectory(fileInfo.isDir());
        
        // 计算相关性分数 (0.0 - 1.0)
        float score = 0.5f;  // 默认中等相关性
        if (result.content.contains("<b>", Qt::CaseInsensitive)) {
            // 根据高亮标记出现的次数计算分数
            int highlightCount = result.content.count("<b>", Qt::CaseInsensitive);
            score = qMin(1.0f, 0.5f + (highlightCount * 0.1f));
        }
        searchResult.setScore(score);
        
        // 添加自定义属性
        searchResult.setCustomAttribute("matchCount", result.content.count("<b>", Qt::CaseInsensitive));
        searchResult.setCustomAttribute("fileType", fileInfo.suffix());
        
        convertedResults.append(searchResult);
    }
    
    return convertedResults;
}

}  // namespace Search
}  // namespace DFM6 