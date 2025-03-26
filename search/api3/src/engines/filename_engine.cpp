#include "qsearch/engines/filename_engine.h"
#include "qsearch/search_options.h"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <functional>
#include <QRegularExpression>

namespace QSearch {

struct FilenameSearchEngine::Impl {
    SearchQuery query;
    SearchOptions options;
    SearchState currentState = SearchState::Idle;
    SearchResult results;
    std::atomic<double> progress{0.0};
    QTimer* progressTimer = nullptr;
    bool useIndex = false;
    bool stopRequested = false;
    QSqlDatabase db;
    
    // 索引搜索
    bool searchWithIndex() {
        // 假设我们已经有一个索引数据库
        if (!db.isOpen()) {
            // 尝试打开索引数据库
            QString dbName = "qsearch_" + QString::number(static_cast<int>(IndexType::Filename));
            if (QSqlDatabase::contains(dbName)) {
                db = QSqlDatabase::database(dbName);
            } else {
                // 索引数据库未初始化
                return false;
            }
        }
        
        // 构建SQL查询
        QString sql = "SELECT id, name, path, size, modified_time, is_directory FROM files WHERE ";
        
        QString whereClause;
        QStringList values;
        
        // 根据匹配类型构建查询条件
        switch (query.matchType()) {
            case MatchType::Exact:
                whereClause = "name = ?";
                values << query.text();
                break;
            case MatchType::Contains:
                whereClause = "name LIKE ?";
                values << "%" + query.text() + "%";
                break;
            case MatchType::StartsWith:
                whereClause = "name LIKE ?";
                values << query.text() + "%";
                break;
            case MatchType::EndsWith:
                whereClause = "name LIKE ?";
                values << "%" + query.text();
                break;
            case MatchType::Regex:
                // SQLite不直接支持正则，这里简化为LIKE
                whereClause = "name LIKE ?";
                values << "%" + query.text() + "%";
                break;
            case MatchType::Fuzzy:
                // 简化实现，实际应使用编辑距离算法
                whereClause = "name LIKE ?";
                values << "%" + query.text() + "%";
                break;
            case MatchType::Pinyin:
                // 拼音搜索需要特殊实现
                whereClause = "name LIKE ?";
                values << "%" + query.text() + "%";
                break;
        }
        
        // 添加路径限制
        if (!query.paths().isEmpty()) {
            QStringList pathClauses;
            for (const QString& path : query.paths()) {
                pathClauses << "(path = ? OR path LIKE ?)";
                values << path << path + "/%";
            }
            whereClause += " AND (" + pathClauses.join(" OR ") + ")";
        }
        
        // 添加文件过滤
        if (!query.fileFilters().isEmpty()) {
            QStringList filterClauses;
            for (const QString& filter : query.fileFilters()) {
                if (filter.startsWith("*.")) {
                    QString ext = filter.mid(1);
                    filterClauses << "name LIKE ?";
                    values << "%" + ext;
                }
            }
            if (!filterClauses.isEmpty()) {
                whereClause += " AND (" + filterClauses.join(" OR ") + ")";
            }
        }
        
        // 执行查询
        sql += whereClause;
        
        // 添加限制
        if (options.maxResults() > 0) {
            sql += " LIMIT " + QString::number(options.maxResults());
        }
        
        QSqlQuery sqlQuery(db);
        sqlQuery.prepare(sql);
        
        // 绑定参数
        for (const QString& value : values) {
            sqlQuery.addBindValue(value);
        }
        
        if (!sqlQuery.exec()) {
            return false;
        }
        
        // 处理查询结果
        while (sqlQuery.next() && !stopRequested) {
            ResultItem item;
            item.name = sqlQuery.value("name").toString();
            item.path = sqlQuery.value("path").toString();
            item.url = QUrl::fromLocalFile(item.path);
            item.size = sqlQuery.value("size").toLongLong();
            item.modifiedTime = QDateTime::fromSecsSinceEpoch(sqlQuery.value("modified_time").toInt());
            item.isDirectory = sqlQuery.value("is_directory").toBool();
            
            // 添加匹配信息
            if (item.name.contains(query.text(), Qt::CaseInsensitive)) {
                ResultItem::MatchInfo matchInfo;
                matchInfo.position = item.name.indexOf(query.text(), 0, Qt::CaseInsensitive);
                matchInfo.length = query.text().length();
                item.matches.append(matchInfo);
            }
            
            // 添加结果并发送信号
            results.addItem(item);
            emit resultItemFound(item);
            
            // 更新进度
            progress = 0.5; // 假设进度为50%，因为我们不知道总数
        }
        
        return true;
    }
    
    // 遍历搜索
    bool searchWithTraversal() {
        // 确定搜索路径
        QStringList searchPaths = query.paths();
        if (searchPaths.isEmpty()) {
            searchPaths << QDir::homePath(); // 默认搜索家目录
        }
        
        // 设置文件过滤器
        QStringList nameFilters = query.fileFilters();
        if (nameFilters.isEmpty()) {
            nameFilters << "*"; // 默认匹配所有文件
        }
        
        // 编译正则表达式（如果需要）
        QRegularExpression regex;
        if (query.matchType() == MatchType::Regex) {
            regex = QRegularExpression(query.text(), 
                     options.isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            
            if (!regex.isValid()) {
                return false;
            }
        }
        
        // 遍历每个搜索路径
        int totalFiles = 0;
        int processedFiles = 0;
        
        // 遍历目录计算总文件数（用于进度计算）
        for (const QString& path : searchPaths) {
            QDirIterator it(path, nameFilters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, 
                            QDirIterator::Subdirectories);
            while (it.hasNext() && !stopRequested) {
                it.next();
                totalFiles++;
            }
        }
        
        // 执行实际搜索
        for (const QString& path : searchPaths) {
            QDirIterator it(path, nameFilters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, 
                            QDirIterator::Subdirectories);
            
            while (it.hasNext() && !stopRequested) {
                it.next();
                processedFiles++;
                
                QFileInfo fileInfo = it.fileInfo();
                QString fileName = fileInfo.fileName();
                
                // 根据匹配类型进行匹配
                bool matches = false;
                int matchPosition = -1;
                int matchLength = 0;
                
                switch (query.matchType()) {
                    case MatchType::Exact:
                        matches = (fileName.compare(query.text(), 
                                  options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0);
                        matchPosition = 0;
                        matchLength = fileName.length();
                        break;
                        
                    case MatchType::Contains:
                        matchPosition = fileName.indexOf(query.text(), 0, 
                                       options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
                        matches = (matchPosition >= 0);
                        matchLength = query.text().length();
                        break;
                        
                    case MatchType::StartsWith:
                        matches = fileName.startsWith(query.text(), 
                                 options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
                        matchPosition = 0;
                        matchLength = query.text().length();
                        break;
                        
                    case MatchType::EndsWith:
                        matches = fileName.endsWith(query.text(), 
                                 options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
                        matchPosition = fileName.length() - query.text().length();
                        matchLength = query.text().length();
                        break;
                        
                    case MatchType::Regex:
                        {
                            auto match = regex.match(fileName);
                            matches = match.hasMatch();
                            if (matches) {
                                matchPosition = match.capturedStart();
                                matchLength = match.capturedLength();
                            }
                        }
                        break;
                        
                    case MatchType::Fuzzy:
                        // 简单的模糊匹配实现
                        matches = fileName.contains(query.text(), 
                                 options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
                        matchPosition = fileName.indexOf(query.text(), 0, 
                                      options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
                        matchLength = query.text().length();
                        break;
                        
                    case MatchType::Pinyin:
                        // 拼音搜索需要特殊实现，这里简化
                        matches = fileName.contains(query.text(), Qt::CaseInsensitive);
                        matchPosition = fileName.indexOf(query.text(), 0, Qt::CaseInsensitive);
                        matchLength = query.text().length();
                        break;
                }
                
                // 如果匹配成功，添加到结果中
                if (matches) {
                    ResultItem item;
                    item.name = fileName;
                    item.path = fileInfo.absoluteFilePath();
                    item.url = QUrl::fromLocalFile(item.path);
                    item.size = fileInfo.size();
                    item.modifiedTime = fileInfo.lastModified();
                    item.isDirectory = fileInfo.isDir();
                    
                    // 添加匹配信息
                    if (matchPosition >= 0) {
                        ResultItem::MatchInfo matchInfo;
                        matchInfo.position = matchPosition;
                        matchInfo.length = matchLength;
                        item.matches.append(matchInfo);
                    }
                    
                    // 添加结果并发送信号
                    results.addItem(item);
                    emit resultItemFound(item);
                    
                    // 检查是否达到最大结果数
                    if (options.maxResults() > 0 && results.count() >= options.maxResults()) {
                        break;
                    }
                }
                
                // 更新进度
                if (totalFiles > 0) {
                    progress = static_cast<double>(processedFiles) / totalFiles;
                }
            }
            
            if (stopRequested || (options.maxResults() > 0 && results.count() >= options.maxResults())) {
                break;
            }
        }
        
        return true;
    }
};

FilenameSearchEngine::FilenameSearchEngine(QObject* parent) : SearchEngine(parent), d(new Impl) {
    // 初始化进度定时器
    d->progressTimer = new QTimer(this);
    connect(d->progressTimer, &QTimer::timeout, this, [this]() {
        emit progressChanged(d->progress);
    });
}

FilenameSearchEngine::~FilenameSearchEngine() {
    if (d->progressTimer) {
        d->progressTimer->stop();
    }
}

QString FilenameSearchEngine::engineId() const {
    return "filename";
}

QString FilenameSearchEngine::engineName() const {
    return "文件名搜索引擎";
}

QString FilenameSearchEngine::engineDescription() const {
    return "搜索文件名或目录名的引擎";
}

bool FilenameSearchEngine::supportsIndexing() const {
    return true;
}

bool FilenameSearchEngine::supportsQueryType(QueryType type) const {
    return type == QueryType::Filename || type == QueryType::Both;
}

bool FilenameSearchEngine::supportsMatchType(MatchType type) const {
    return true; // 支持所有匹配类型
}

bool FilenameSearchEngine::supportsFeature(const QString& feature) const {
    // 检查是否支持特定功能
    if (feature == Features::PINYIN_SEARCH) {
        return false; // 暂不支持拼音搜索
    }
    else if (feature == Features::FUZZY_SEARCH) {
        return true;
    }
    else if (feature == Features::REGEX_SEARCH) {
        return true;
    }
    
    return false;
}

bool FilenameSearchEngine::prepare(const SearchQuery& query, const SearchOptions& options) {
    // 准备搜索
    d->query = query;
    d->options = options;
    d->currentState = SearchState::Idle;
    d->results = SearchResult();
    d->progress = 0.0;
    d->stopRequested = false;
    
    // 决定使用索引还是直接搜索
    d->useIndex = (options.searchMode() == SearchMode::Indexed || 
                  options.searchMode() == SearchMode::Fast);
    
    return true;
}

bool FilenameSearchEngine::start() {
    if (d->currentState == SearchState::Searching) {
        return false; // 已经在搜索中
    }
    
    d->currentState = SearchState::Searching;
    d->stopRequested = false;
    emit stateChanged(d->currentState);
    
    // 开始进度更新
    d->progressTimer->start(100);
    
    // 执行搜索
    bool success = false;
    
    try {
        if (d->useIndex) {
            success = d->searchWithIndex();
            if (!success) {
                // 索引搜索失败，回退到直接搜索
                success = d->searchWithTraversal();
            }
        } else {
            success = d->searchWithTraversal();
        }
    } catch (const std::exception& e) {
        setErrorString(QString("搜索异常：%1").arg(e.what()));
        d->currentState = SearchState::Error;
        emit stateChanged(d->currentState);
        d->progressTimer->stop();
        return false;
    }
    
    // 搜索完成
    d->progressTimer->stop();
    
    if (d->stopRequested) {
        d->currentState = SearchState::Idle;
    } else if (!success) {
        d->currentState = SearchState::Error;
    } else {
        d->currentState = SearchState::Completed;
        emit searchCompleted();
    }
    
    emit stateChanged(d->currentState);
    return success;
}

bool FilenameSearchEngine::pause() {
    if (d->currentState != SearchState::Searching) {
        return false;
    }
    
    d->currentState = SearchState::Paused;
    emit stateChanged(d->currentState);
    d->progressTimer->stop();
    
    return true;
}

bool FilenameSearchEngine::resume() {
    if (d->currentState != SearchState::Paused) {
        return false;
    }
    
    d->currentState = SearchState::Searching;
    emit stateChanged(d->currentState);
    d->progressTimer->start(100);
    
    return true;
}

bool FilenameSearchEngine::stop() {
    if (d->currentState != SearchState::Searching && d->currentState != SearchState::Paused) {
        return false;
    }
    
    d->stopRequested = true;
    d->currentState = SearchState::Idle;
    emit stateChanged(d->currentState);
    d->progressTimer->stop();
    
    return true;
}

SearchState FilenameSearchEngine::state() const {
    return d->currentState;
}

double FilenameSearchEngine::progress() const {
    return d->progress;
}

SearchResult FilenameSearchEngine::currentResults() const {
    return d->results;
}

} // namespace QSearch 