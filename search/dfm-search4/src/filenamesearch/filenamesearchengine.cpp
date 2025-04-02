#include "filenamesearchengine.h"
#include <QtConcurrent>
#include <QFileInfo>
#include <QDebug>

// 前向声明的类型定义，实际项目中可能需要替换为真实的结构
class LuceneSearchEngine;
struct FileData {
    QString filePath;
    qint64 size;
    QDateTime modifiedTime;
    bool isDirectory;
};

namespace DFM6 {
namespace Search {

FileNameSearchEngine::FileNameSearchEngine(QObject *parent)
    : AbstractSearchEngine(parent),
      m_engine(nullptr)
{
    // 实际项目中这里应初始化 m_engine
    // m_engine = std::make_unique<LuceneSearchEngine>();
}

FileNameSearchEngine::~FileNameSearchEngine() = default;

SearchOptions FileNameSearchEngine::searchOptions() const
{
    return m_options;
}

void FileNameSearchEngine::setSearchOptions(const SearchOptions &options)
{
    // 尝试将基类选项转换为 FileNameSearchOptions
    try {
        const FileNameSearchOptions* fileNameOptions = dynamic_cast<const FileNameSearchOptions*>(&options);
        if (fileNameOptions) {
            m_options = *fileNameOptions;
        } else {
            m_options = FileNameSearchOptions(options);
        }
    } catch (const std::exception& e) {
        qWarning() << "Failed to convert search options:" << e.what();
        m_options = FileNameSearchOptions(options);
    }
    
    // 配置底层引擎
    if (m_engine) {
        // TODO: 配置 m_engine 的选项
    }
}

SearchStatus FileNameSearchEngine::status() const
{
    return m_status.load();
}

QFuture<QList<SearchResult>> FileNameSearchEngine::search(const SearchQuery &query)
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

QFuture<void> FileNameSearchEngine::searchWithCallback(const SearchQuery &query, 
                                                     SearchEngine::ResultCallback callback)
{
    if (m_status.load() == SearchStatus::Searching) {
        return QtConcurrent::run([]() {});
    }
    
    m_cancelled.store(false);
    setStatus(SearchStatus::Searching);
    emit searchStarted();
    
    return QtConcurrent::run([this, query, callback]() {
        // 假设这是一个虚拟实现，实际项目中应使用真实的搜索功能
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

QList<SearchResult> FileNameSearchEngine::searchSync(const SearchQuery &query)
{
    // 注：这里只是演示，实际项目中应使用 LuceneSearchEngine 实现
    QList<SearchResult> results;
    
    if (m_cancelled.load()) {
        return results;
    }
    
    try {
        QString queryStr = convertQuery(query);
        
        // 假设的搜索结果
        QVector<FileData> fakeResults;
        
        // 在实际实现中，这里应调用 m_engine 进行搜索
        // if (m_engine) {
        //     fakeResults = m_engine->search(queryStr, m_options.searchPath());
        // }
        
        // 模拟一些搜索结果
        FileData fakeFile;
        fakeFile.filePath = m_options.searchPath() + "/example.txt";
        fakeFile.size = 1024;
        fakeFile.modifiedTime = QDateTime::currentDateTime();
        fakeFile.isDirectory = false;
        fakeResults.append(fakeFile);
        
        return convertResults(fakeResults);
    } 
    catch (const std::exception& e) {
        qWarning() << "Search error:" << e.what();
        reportError(QString("Search error: %1").arg(e.what()));
        return QList<SearchResult>();
    }
}

void FileNameSearchEngine::pause()
{
    if (m_status.load() == SearchStatus::Searching) {
        setStatus(SearchStatus::Paused);
    }
}

void FileNameSearchEngine::resume()
{
    if (m_status.load() == SearchStatus::Paused) {
        m_mutex.lock();
        setStatus(SearchStatus::Searching);
        m_pauseCondition.wakeAll();
        m_mutex.unlock();
    }
}

void FileNameSearchEngine::cancel()
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

void FileNameSearchEngine::clearCache()
{
    // 在实际实现中，这里应清除 m_engine 的缓存
    // if (m_engine) {
    //     m_engine->clearCache();
    // }
}

QString FileNameSearchEngine::convertQuery(const SearchQuery &query)
{
    // 实际实现应根据 query 的类型构建适合底层引擎的查询
    return query.keyword();
}

QList<SearchResult> FileNameSearchEngine::convertResults(const QVector<FileData> &fileDataList)
{
    QList<SearchResult> results;
    
    for (const auto &fileData : fileDataList) {
        SearchResult result(fileData.filePath);
        result.setSize(fileData.size);
        result.setModifiedTime(fileData.modifiedTime);
        result.setIsDirectory(fileData.isDirectory);
        
        // 可以设置其他自定义属性
        result.setScore(1.0f); // 默认分数
        
        results.append(result);
    }
    
    return results;
}

}  // namespace Search
}  // namespace DFM6 