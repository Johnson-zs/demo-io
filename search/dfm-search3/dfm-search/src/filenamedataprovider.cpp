#include "dfm-search/filenamedataprovider.h"
#include "dfm-search/searchresult.h"
#include <QDir>
#include <QDirIterator>
#include <QThread>
#include <QRegularExpression>
#include <QDebug>

namespace DFMSearch {

// 搜索工作线程类
class FileNameSearchWorker : public QObject {
    Q_OBJECT
public:
    explicit FileNameSearchWorker(const SearchQuery& query, QObject* parent = nullptr)
        : QObject(parent), m_query(query), m_stop(false) {}

public slots:
    void startSearch() {
        m_stop = false;
        const QStringList paths = m_query.searchPaths();
        
        if (paths.isEmpty()) {
            emit error(QStringLiteral("没有指定搜索路径"));
            emit finished();
            return;
        }
        
        const bool caseSensitive = m_query.hasFlag(SearchFlag::CaseSensitive);
        const bool regexSupport = m_query.hasFlag(SearchFlag::RegexSupport);
        const bool fuzzyMatch = m_query.hasFlag(SearchFlag::FuzzyMatch);
        
        int resultCount = 0;
        int limit = m_query.limit();
        
        for (const auto& path : paths) {
            if (m_stop) break;
            
            QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, 
                            QDirIterator::Subdirectories);
            
            int fileCount = 0;
            int totalFiles = 1000; // 估计值，仅用于进度显示
            
            while (it.hasNext() && !m_stop) {
                const QString filePath = it.next();
                const QString fileName = it.fileName();
                
                ++fileCount;
                if (fileCount % 100 == 0) {
                    emit progress(qMin(fileCount * 100 / totalFiles, 99));
                }
                
                bool isMatch = false;
                if (regexSupport) {
                    // 正则表达式匹配
                    QRegularExpression regex(m_query.keyword(), 
                        caseSensitive ? QRegularExpression::NoPatternOption 
                                      : QRegularExpression::CaseInsensitiveOption);
                    isMatch = regex.match(fileName).hasMatch();
                } else if (fuzzyMatch) {
                    // 模糊匹配（简单实现：包含关键词）
                    if (caseSensitive) {
                        isMatch = fileName.contains(m_query.keyword());
                    } else {
                        isMatch = fileName.toLower().contains(m_query.keyword().toLower());
                    }
                } else {
                    // 精确匹配
                    if (caseSensitive) {
                        isMatch = fileName == m_query.keyword();
                    } else {
                        isMatch = fileName.toLower() == m_query.keyword().toLower();
                    }
                }
                
                if (isMatch) {
                    QFileInfo fileInfo(filePath);
                    
                    FileSearchResult result;
                    result.setTitle(fileName);
                    result.setPath(filePath);
                    result.setFileSize(fileInfo.size());
                    result.setModifiedTime(fileInfo.lastModified());
                    
                    emit resultFound(result);
                    
                    ++resultCount;
                    if (limit > 0 && resultCount >= limit) {
                        break;
                    }
                }
            }
            
            if (m_stop) break;
        }
        
        emit progress(100);
        emit finished();
    }
    
    void stopSearch() {
        m_stop = true;
    }

signals:
    void resultFound(const SearchResult& result);
    void progress(int percentage);
    void error(const QString& errorMessage);
    void finished();

private:
    SearchQuery m_query;
    bool m_stop;
};

// 私有实现类
class FileNameDataProviderPrivate {
public:
    QThread* workerThread = nullptr;
    FileNameSearchWorker* worker = nullptr;
};

FileNameDataProvider::FileNameDataProvider(QObject* parent)
    : SearchProvider(parent)
    , d_ptr(std::make_unique<FileNameDataProviderPrivate>())
{
}

FileNameDataProvider::~FileNameDataProvider()
{
    stop();
}

QString FileNameDataProvider::name() const
{
    return QStringLiteral("文件名搜索");
}

QString FileNameDataProvider::description() const
{
    return QStringLiteral("在文件系统中搜索匹配文件名的文件");
}

SearchType FileNameDataProvider::searchType() const
{
    return SearchType::FileName;
}

SearchMechanism FileNameDataProvider::mechanism() const
{
    return SearchMechanism::RealTime;
}

bool FileNameDataProvider::stop()
{
    if (!SearchProvider::stop()) {
        return false;
    }
    
    if (d_ptr->worker) {
        d_ptr->worker->stopSearch();
    }
    
    if (d_ptr->workerThread) {
        d_ptr->workerThread->quit();
        d_ptr->workerThread->wait();
        
        delete d_ptr->workerThread;
        d_ptr->workerThread = nullptr;
        d_ptr->worker = nullptr;
    }
    
    return true;
}

bool FileNameDataProvider::doSearch()
{
    if (d_ptr->workerThread) {
        // 如果已有线程在运行，先停止
        stop();
    }
    
    // 创建新的工作线程
    d_ptr->workerThread = new QThread;
    d_ptr->worker = new FileNameSearchWorker(query());
    d_ptr->worker->moveToThread(d_ptr->workerThread);
    
    connect(d_ptr->workerThread, &QThread::started, 
            d_ptr->worker, &FileNameSearchWorker::startSearch);
    connect(d_ptr->worker, &FileNameSearchWorker::finished, 
            this, &FileNameDataProvider::onWorkerFinished);
    connect(d_ptr->worker, &FileNameSearchWorker::resultFound, 
            this, &FileNameDataProvider::onWorkerResult);
    connect(d_ptr->worker, &FileNameSearchWorker::progress, 
            this, &FileNameDataProvider::onWorkerProgress);
    connect(d_ptr->worker, &FileNameSearchWorker::error, 
            this, &FileNameDataProvider::onWorkerError);
    connect(d_ptr->worker, &FileNameSearchWorker::finished, 
            d_ptr->workerThread, &QThread::quit);
    connect(d_ptr->worker, &FileNameSearchWorker::finished, 
            d_ptr->worker, &FileNameSearchWorker::deleteLater);
    connect(d_ptr->workerThread, &QThread::finished, 
            d_ptr->workerThread, &QThread::deleteLater);
    
    d_ptr->workerThread->start();
    return true;
}

void FileNameDataProvider::onWorkerFinished()
{
    reportCompleted();
    d_ptr->worker = nullptr;
    d_ptr->workerThread = nullptr;
}

void FileNameDataProvider::onWorkerResult(const SearchResult& result)
{
    addResult(result);
}

void FileNameDataProvider::onWorkerProgress(int progress)
{
    setProgress(progress);
}

void FileNameDataProvider::onWorkerError(const QString& error)
{
    reportError(error);
}

} // namespace DFMSearch

#include "filenamedataprovider.moc" 