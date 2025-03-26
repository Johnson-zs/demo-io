#include "qsearch/engines/fulltext_engine.h"
#include "qsearch/search_options.h"
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QRegularExpression>
#include <atomic>

namespace QSearch {

struct FulltextSearchEngine::Impl {
    SearchQuery query;
    SearchOptions options;
    SearchState currentState = SearchState::Idle;
    SearchResult results;
    std::atomic<double> progress{0.0};
    QTimer* progressTimer = nullptr;
    bool useIndex = false;
    bool stopRequested = false;
    QSqlDatabase db;
    
    // 索引搜索实现
    bool searchWithIndex() {
        // 尝试打开索引数据库
        if (!db.isOpen()) {
            QString dbName = "qsearch_" + QString::number(static_cast<int>(IndexType::FileContent));
            if (QSqlDatabase::contains(dbName)) {
                db = QSqlDatabase::database(dbName);
            } else {
                // 索引数据库未初始化
                return false;
            }
        }
        
        // 构建SQL查询
        QStringList terms = query.text().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (terms.isEmpty()) {
            return true; // 空查询返回空结果
        }
        
        // 基本查询
        QString sql = "SELECT f.id, f.name, f.path, f.size, f.modified_time, f.is_directory "
                     "FROM files f "
                     "JOIN content_index ci ON f.id = ci.file_id "
                     "WHERE ";
        
        // 根据路径限制构建查询条件
        QStringList pathClauses;
        if (!query.paths().isEmpty()) {
            for (const QString& path : query.paths()) {
                pathClauses << "(f.path = ? OR f.path LIKE ?)";
            }
        }
        
        // 根据搜索词构建查询条件
        QStringList termClauses;
        for (const QString& term : terms) {
            termClauses << "ci.term LIKE ?";
        }
        
        // 组合所有条件
        QStringList conditions;
        if (!pathClauses.isEmpty()) {
            conditions << "(" + pathClauses.join(" OR ") + ")";
        }
        if (!termClauses.isEmpty()) {
            conditions << "(" + termClauses.join(" OR ") + ")";
        }
        
        sql += conditions.join(" AND ");
        sql += " GROUP BY f.id";
        
        // 添加限制
        if (options.maxResults() > 0) {
            sql += " LIMIT " + QString::number(options.maxResults());
        }
        
        QSqlQuery sqlQuery(db);
        sqlQuery.prepare(sql);
        
        // 绑定参数值
        if (!query.paths().isEmpty()) {
            for (const QString& path : query.paths()) {
                sqlQuery.addBindValue(path);
                sqlQuery.addBindValue(path + "/%");
            }
        }
        
        for (const QString& term : terms) {
            sqlQuery.addBindValue("%" + term + "%");
        }
        
        if (!sqlQuery.exec()) {
            return false;
        }
        
        // 处理结果
        while (sqlQuery.next()) {
            if (stopRequested) {
                break;
            }
            
            ResultItem item;
            item.name = sqlQuery.value("name").toString();
            item.path = sqlQuery.value("path").toString();
            item.url = QUrl::fromLocalFile(item.path);
            item.size = sqlQuery.value("size").toLongLong();
            item.modifiedTime = sqlQuery.value("modified_time").toDateTime();
            item.isDirectory = sqlQuery.value("is_directory").toBool();
            
            // 获取匹配内容上下文（需要额外查询）
            QSqlQuery contextQuery(db);
            contextQuery.prepare("SELECT position, length, context FROM content_match "
                               "WHERE file_id = ? AND term = ? LIMIT 10");
            contextQuery.addBindValue(sqlQuery.value("id"));
            
            for (const QString& term : terms) {
                contextQuery.addBindValue(term);
                if (contextQuery.exec()) {
                    while (contextQuery.next()) {
                        ResultItem::MatchInfo match;
                        match.position = contextQuery.value("position").toInt();
                        match.length = contextQuery.value("length").toInt();
                        match.context = contextQuery.value("context").toString();
                        item.matches.append(match);
                    }
                }
            }
            
            results.addItem(item);
            emit resultItemFound(item);
        }
        
        return true;
    }
    
    // 直接文件内容搜索
    bool searchWithDirectSearch() {
        // 获取所有要搜索的路径
        QStringList paths = query.paths();
        if (paths.isEmpty()) {
            paths << QDir::homePath();
        }
        
        // 解析文件过滤器
        QStringList nameFilters;
        if (!query.fileFilters().isEmpty()) {
            nameFilters = query.fileFilters();
        } else {
            // 默认搜索常见文本文件
            nameFilters << "*.txt" << "*.md" << "*.cpp" << "*.h" << "*.py" << "*.xml" << "*.json";
        }
        
        // 准备正则表达式
        QRegularExpression regex;
        if (query.matchType() == MatchType::Regex) {
            regex = QRegularExpression(query.text());
            if (!regex.isValid()) {
                return false;
            }
        }
        
        // 遍历所有目录
        int totalFilesSearched = 0;
        int totalFilesEstimate = 1000; // 估计值
        
        for (const QString& path : paths) {
            QDirIterator it(path, nameFilters, 
                           QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                           QDirIterator::Subdirectories);
            
            while (it.hasNext()) {
                if (stopRequested) {
                    return true;
                }
                
                QString filePath = it.next();
                QFileInfo fileInfo(filePath);
                
                // 更新进度
                totalFilesSearched++;
                progress = static_cast<double>(totalFilesSearched) / totalFilesEstimate;
                
                // 检查文件大小限制
                if (fileInfo.size() > 10 * 1024 * 1024) { // 跳过大于10MB的文件
                    continue;
                }
                
                // 搜索文件内容
                QFile file(filePath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    continue;
                }
                
                QTextStream in(&file);
                QString content = in.readAll();
                file.close();
                
                bool found = false;
                QVector<ResultItem::MatchInfo> matches;
                
                // 根据匹配类型搜索
                if (query.matchType() == MatchType::Regex) {
                    auto matchIterator = regex.globalMatch(content);
                    while (matchIterator.hasNext()) {
                        auto match = matchIterator.next();
                        ResultItem::MatchInfo info;
                        info.position = match.capturedStart();
                        info.length = match.capturedLength();
                        
                        // 提取上下文
                        int contextStart = qMax(0, info.position - 50);
                        int contextEnd = qMin(content.length(), info.position + info.length + 50);
                        info.context = content.mid(contextStart, contextEnd - contextStart);
                        
                        matches.append(info);
                        found = true;
                    }
                } else {
                    // 简单的文本搜索
                    int pos = 0;
                    QString searchText = query.text();
                    Qt::CaseSensitivity cs = options.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
                    
                    while ((pos = content.indexOf(searchText, pos, cs)) != -1) {
                        ResultItem::MatchInfo info;
                        info.position = pos;
                        info.length = searchText.length();
                        
                        // 提取上下文
                        int contextStart = qMax(0, pos - 50);
                        int contextEnd = qMin(content.length(), pos + info.length + 50);
                        info.context = content.mid(contextStart, contextEnd - contextStart);
                        
                        matches.append(info);
                        found = true;
                        pos += searchText.length();
                    }
                }
                
                if (found) {
                    ResultItem item;
                    item.name = fileInfo.fileName();
                    item.path = fileInfo.absoluteFilePath();
                    item.url = QUrl::fromLocalFile(item.path);
                    item.size = fileInfo.size();
                    item.modifiedTime = fileInfo.lastModified();
                    item.isDirectory = false;
                    item.matches = matches;
                    
                    results.addItem(item);
                    emit resultItemFound(item);
                }
            }
        }
        
        return true;
    }
};

FulltextSearchEngine::FulltextSearchEngine(QObject* parent) : SearchEngine(parent), d(new Impl) {
    d->progressTimer = new QTimer(this);
    connect(d->progressTimer, &QTimer::timeout, this, [this]() {
        emit progressChanged(d->progress);
    });
}

FulltextSearchEngine::~FulltextSearchEngine() {
    if (d->progressTimer) {
        d->progressTimer->stop();
    }
}

QString FulltextSearchEngine::engineId() const {
    return "fulltext";
}

QString FulltextSearchEngine::engineName() const {
    return "全文搜索引擎";
}

QString FulltextSearchEngine::engineDescription() const {
    return "搜索文件内容的引擎";
}

bool FulltextSearchEngine::supportsIndexing() const {
    return true;
}

bool FulltextSearchEngine::supportsQueryType(QueryType type) const {
    return type == QueryType::FileContent || type == QueryType::Both;
}

bool FulltextSearchEngine::supportsMatchType(MatchType type) const {
    return type == MatchType::Contains || type == MatchType::Exact || type == MatchType::Regex;
}

bool FulltextSearchEngine::supportsFeature(const QString& feature) const {
    if (feature == Features::CONTENT_PREVIEW) {
        return true;
    }
    else if (feature == Features::REGEX_SEARCH) {
        return true;
    }
    return false;
}

bool FulltextSearchEngine::prepare(const SearchQuery& query, const SearchOptions& options) {
    // 准备搜索
    d->query = query;
    d->options = options;
    d->currentState = SearchState::Idle;
    d->results = SearchResult();
    d->progress = 0.0;
    d->stopRequested = false;
    
    // 决定使用索引还是直接搜索
    d->useIndex = (options.searchMode() == SearchMode::Indexed);
    
    return true;
}

bool FulltextSearchEngine::start() {
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
                success = d->searchWithDirectSearch();
            }
        } else {
            success = d->searchWithDirectSearch();
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

bool FulltextSearchEngine::pause() {
    if (d->currentState != SearchState::Searching) {
        return false;
    }
    
    d->currentState = SearchState::Paused;
    emit stateChanged(d->currentState);
    d->progressTimer->stop();
    
    return true;
}

bool FulltextSearchEngine::resume() {
    if (d->currentState != SearchState::Paused) {
        return false;
    }
    
    d->currentState = SearchState::Searching;
    emit stateChanged(d->currentState);
    d->progressTimer->start(100);
    
    return true;
}

bool FulltextSearchEngine::stop() {
    if (d->currentState != SearchState::Searching && d->currentState != SearchState::Paused) {
        return false;
    }
    
    d->stopRequested = true;
    d->currentState = SearchState::Idle;
    emit stateChanged(d->currentState);
    d->progressTimer->stop();
    
    return true;
}

SearchState FulltextSearchEngine::state() const {
    return d->currentState;
}

double FulltextSearchEngine::progress() const {
    return d->progress;
}

SearchResult FulltextSearchEngine::currentResults() const {
    return d->results;
}

} // namespace QSearch 