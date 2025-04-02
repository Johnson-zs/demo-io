#include "filenamesearchengine.h"
#include <QtConcurrent>
#include <QFileInfo>
#include <QDebug>
#include <QDirIterator>

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
    // 不再需要转换类型
    m_options = options;
    
    // 配置底层引擎
    if (m_engine) {
        // 直接使用基类接口
        bool pinyinEnabled = options.pinyinEnabled();
        bool fuzzySearch = options.fuzzySearch();
        QStringList fileTypes = options.fileTypes();
        
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
        const SearchMethod method = m_options.method();
        
        if (method == SearchMethod::Realtime) {
            // 实时搜索实现
            QDir dir(m_options.searchPath());
            QStringList filters = m_options.fileTypes();
            QDirIterator::IteratorFlags flags = m_options.isRecursive() ? 
                QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
            
            QDirIterator it(dir.path(), filters, QDir::Files | QDir::NoDotAndDotDot, flags);
            int count = 0;
            
            while (it.hasNext() && !m_cancelled.load()) {
                // 暂停处理
                processPauseIfNeeded();
                
                QString filePath = it.next();
                QFileInfo info(filePath);
                
                // 检查是否符合搜索条件
                if (matchesQuery(info, query)) {
                    count++;
                    SearchResult result(filePath);
                    result.setSize(info.size());
                    result.setModifiedTime(info.lastModified());
                    result.setIsDirectory(info.isDir());
                    
                    // 立即回调返回结果
                    if (callback) {
                        callback(result);
                    }
                    
                    emit resultFound(result);
                }
                
                // 每处理10个文件报告一次进度
                if (count % 10 == 0) {
                    emit progressChanged(count, -1); // -1表示总数未知
                }
            }
            
            // 搜索完成
            if (!m_cancelled.load()) {
                setStatus(SearchStatus::Finished);
                QList<SearchResult> emptyList; // 实时模式下不积累结果
                emit searchFinished(emptyList);
            }
        } else {
            // 索引搜索实现（保持原有代码）
            QList<SearchResult> results = searchSync(query);
            
            if (!m_cancelled.load()) {
                setStatus(SearchStatus::Finished);
                emit searchFinished(results);
            }
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
        
        // 获取文件类型过滤器
        QStringList fileTypes = m_options.fileTypes();
        bool hasKeyword = !query.keyword().isEmpty() && query.keyword() != "*";
        bool hasFileTypes = !fileTypes.isEmpty();

        // 如果只有文件类型没有关键词，使用特殊的搜索逻辑
        if (hasFileTypes && !hasKeyword) {
            // 按类型搜索代码
            // ...
        }
        
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