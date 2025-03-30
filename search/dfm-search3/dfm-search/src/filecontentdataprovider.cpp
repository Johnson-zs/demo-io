#include "dfm-search/filecontentdataprovider.h"
#include "dfm-search/searchresult.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QRegularExpression>
#include <QDebug>

namespace DFMSearch {

// 搜索工作线程类
class FileContentSearchWorker : public QObject {
    Q_OBJECT
public:
    explicit FileContentSearchWorker(const SearchQuery& query, const QStringList& fileFilters, QObject* parent = nullptr)
        : QObject(parent), m_query(query), m_fileFilters(fileFilters), m_stop(false) {}

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
        
        int resultCount = 0;
        int limit = m_query.limit();
        
        for (const auto& path : paths) {
            if (m_stop) break;
            
            QDirIterator it(path, m_fileFilters.isEmpty() ? QStringList{"*.*"} : m_fileFilters, 
                            QDir::Files, QDirIterator::Subdirectories);
            
            int fileCount = 0;
            int totalFiles = 1000; // 估计值，仅用于进度显示
            
            while (it.hasNext() && !m_stop) {
                const QString filePath = it.next();
                
                ++fileCount;
                if (fileCount % 10 == 0) {
                    emit progress(qMin(fileCount * 100 / totalFiles, 99));
                }
                
                QFile file(filePath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    continue;
                }
                
                QTextStream in(&file);
                int lineNumber = 0;
                
                while (!in.atEnd() && !m_stop) {
                    const QString line = in.readLine();
                    ++lineNumber;
                    
                    bool isMatch = false;
                    QString matchedContent = line;
                    
                    if (regexSupport) {
                        // 正则表达式匹配
                        QRegularExpression regex(m_query.keyword(), 
                            caseSensitive ? QRegularExpression::NoPatternOption 
                                          : QRegularExpression::CaseInsensitiveOption);
                        isMatch = regex.match(line).hasMatch();
                    } else {
                        // 普通文本匹配
                        if (caseSensitive) {
                            isMatch = line.contains(m_query.keyword());
                        } else {
                            isMatch = line.toLower().contains(m_query.keyword().toLower());
                        }
                    }
                    
                    if (isMatch) {
                        ContentSearchResult result;
                        result.setTitle(it.fileName());
                        result.setPath(filePath);
                        result.setMatchedContent(line.trimmed());
                        result.setLineNumber(lineNumber);
                        
                        emit resultFound(result);
                        
                        ++resultCount;
                        if (limit > 0 && resultCount >= limit) {
                            break;
                        }
                    }
                }
                
                file.close();
                
                if (m_stop || (limit > 0 && resultCount >= limit)) {
                    break;
                }
            }
            
            if (m_stop || (limit > 0 && resultCount >= limit)) {
                break;
            }
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
    QStringList m_fileFilters;
    bool m_stop;
};

// 私有实现类
class FileContentDataProviderPrivate {
public:
    QThread* workerThread = nullptr;
    FileContentSearchWorker* worker = nullptr;
    QStringList fileFilters;
};

FileContentDataProvider::FileContentDataProvider(QObject* parent)
    : SearchProvider(parent)
    , d_ptr(std::make_unique<FileContentDataProviderPrivate>())
{
    // 设置默认文本文件过滤器
    d_ptr->fileFilters = {"*.txt", "*.md", "*.cpp", "*.h", "*.c", "*.hpp", "*.cc", "*.py", "*.js", "*.html", "*.css", "*.json", "*.xml"};
}

FileContentDataProvider::~FileContentDataProvider()
{
    stop();
}

QString FileContentDataProvider::name() const
{
    return QStringLiteral("文件内容搜索");
}

QString FileContentDataProvider::description() const
{
    return QStringLiteral("在文件系统中搜索文件内容");
}

SearchType FileContentDataProvider::searchType() const
{
    return SearchType::FileContent;
}

SearchMechanism FileContentDataProvider::mechanism() const
{
    return SearchMechanism::RealTime;
}

void FileContentDataProvider::setFileFilters(const QStringList& filters)
{
    d_ptr->fileFilters = filters;
}

QStringList FileContentDataProvider::fileFilters() const
{
    return d_ptr->fileFilters;
}

bool FileContentDataProvider::stop()
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

bool FileContentDataProvider::doSearch()
{
    if (d_ptr->workerThread) {
        // 如果已有线程在运行，先停止
        stop();
    }
    
    // 创建新的工作线程
    d_ptr->workerThread = new QThread;
    d_ptr->worker = new FileContentSearchWorker(query(), d_ptr->fileFilters);
    d_ptr->worker->moveToThread(d_ptr->workerThread);
    
    connect(d_ptr->workerThread, &QThread::started, 
            d_ptr->worker, &FileContentSearchWorker::startSearch);
    connect(d_ptr->worker, &FileContentSearchWorker::finished, 
            this, &FileContentDataProvider::onWorkerFinished);
    connect(d_ptr->worker, &FileContentSearchWorker::resultFound, 
            this, &FileContentDataProvider::onWorkerResult);
    connect(d_ptr->worker, &FileContentSearchWorker::progress, 
            this, &FileContentDataProvider::onWorkerProgress);
    connect(d_ptr->worker, &FileContentSearchWorker::error, 
            this, &FileContentDataProvider::onWorkerError);
    connect(d_ptr->worker, &FileContentSearchWorker::finished, 
            d_ptr->workerThread, &QThread::quit);
    connect(d_ptr->worker, &FileContentSearchWorker::finished, 
            d_ptr->worker, &FileContentSearchWorker::deleteLater);
    connect(d_ptr->workerThread, &QThread::finished, 
            d_ptr->workerThread, &QThread::deleteLater);
    
    d_ptr->workerThread->start();
    return true;
}

void FileContentDataProvider::onWorkerFinished()
{
    reportCompleted();
    d_ptr->worker = nullptr;
    d_ptr->workerThread = nullptr;
}

void FileContentDataProvider::onWorkerResult(const SearchResult& result)
{
    addResult(result);
}

void FileContentDataProvider::onWorkerProgress(int progress)
{
    setProgress(progress);
}

void FileContentDataProvider::onWorkerError(const QString& error)
{
    reportError(error);
}

} // namespace DFMSearch

#include "filecontentdataprovider.moc" 