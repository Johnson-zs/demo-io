#include "lucenesearchengine.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <unistd.h>

using namespace Lucene;

LuceneSearchEngine::LuceneSearchEngine()
    : m_searchCancelled(false)
{
}

void LuceneSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_cachedAllFiles.clear();
    m_lastKeyword.clear();
    m_lastSearchResults.clear();
}

QString LuceneSearchEngine::getIndexDirectory() const
{
    return QString("/run/user/%1/deepin-anything-server").arg(getuid());
}

QString LuceneSearchEngine::getHomeDirectory() const
{
    QString homeDir;

    if (QFileInfo::exists("/data/home")) {
        homeDir = "/data";
    } else if (QFileInfo::exists("/persistent/home")) {
        homeDir = "/persistent";
    }

    homeDir.append(QDir::homePath());

    return homeDir;
}

QStringList LuceneSearchEngine::performLuceneSearch(const QString &originPath, 
                                                  const QString &key, 
                                                  bool nrt,
                                                  bool caseSensitive,
                                                  bool fuzzySearch) const
{
    // 使用缓存：如果搜索关键词与上次相同且搜索选项也相同，直接返回缓存的结果
    if (!nrt && key == m_lastKeyword && !m_lastSearchResults.isEmpty() && 
        caseSensitive == m_lastCaseSensitive && fuzzySearch == m_lastFuzzySearch) {
        return m_lastSearchResults;
    }
    
    // 在方法开始处重置取消标志
    m_searchCancelled = false;
    
    QString keywords = key;
    QString path = originPath;
    if (path.startsWith(QDir::homePath()))
        path.replace(0, QDir::homePath().length(), getHomeDirectory());

    if (keywords.isEmpty()) {
        return {};
    }

    try {
        // 获取索引目录
        QString indexDir = getIndexDirectory();

        // 打开索引目录
        FSDirectoryPtr directory = FSDirectory::open(StringUtils::toUnicode(indexDir.toStdString()));

        // 检查索引是否存在
        if (!IndexReader::indexExists(directory)) {
            qWarning() << "索引不存在:" << indexDir;
            return QStringList();
        }

        // 打开索引读取器
        IndexReaderPtr reader = IndexReader::open(directory, true);
        if (reader->numDocs() == 0) {
            qWarning() << "索引为空，没有文档";
            return QStringList();
        }

        // 创建搜索器
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        int32_t max_results = reader->numDocs();

        // 创建查询解析器
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT,
                                                       L"file_name",
                                                       newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
        parser->setDefaultOperator(QueryParser::AND_OPERATOR);

        // 使用正确的方法调用，增加大小写敏感和模糊搜索参数
        Lucene::QueryPtr query = buildSearchQuery(keywords, caseSensitive, fuzzySearch);

        // 创建过滤器，仅搜索指定路径下的文件
        FilterPtr pathFilter = newLucene<PrefixFilter>(
            newLucene<Term>(L"full_path", StringUtils::toUnicode(path.toStdString())));
        
        // 使用过滤器执行搜索
        TopDocsPtr search_results = searcher->search(query, pathFilter, max_results);

        QStringList results;
        results.reserve(search_results->scoreDocs.size());
        QStringList dirs, files;
        dirs.reserve(search_results->scoreDocs.size() / 2);   // 预估目录数量
        files.reserve(search_results->scoreDocs.size() / 2);   // 预估文件数量

        for (const auto &score_doc : search_results->scoreDocs) {
            // 检查是否取消搜索
            if (m_searchCancelled) {
                return {};
            }

            DocumentPtr doc = searcher->doc(score_doc->doc);
            auto result = QString::fromStdWString(doc->get(L"full_path"));

            // 先进行路径过滤，避免不必要的类型判断
            if (!result.startsWith(path)) continue;

            // 延迟获取类型，只有通过路径过滤的才需要判断
            auto type = QString::fromStdWString(doc->get(L"file_type"));

            // 使用移动语义 + 条件分类
            if (type == "dir") {
                dirs.append(std::move(result));   // 目录列表
            } else {
                files.append(std::move(result));   // 文件列表
            }
        }

        // 合并结果（O(1) 时间复杂度操作）
        results = std::move(dirs) + std::move(files);

        // 存储结果到缓存，包括搜索参数
        if (!nrt) {
            m_lastKeyword = key;
            m_lastSearchResults = results;
            m_lastCaseSensitive = caseSensitive;
            m_lastFuzzySearch = fuzzySearch;
        }
        
        return results;
    } catch (const LuceneException &e) {
        qWarning() << "Lucene搜索异常:" << QString::fromStdWString(e.getError());
        return {};
    }
}

QVector<FileData> LuceneSearchEngine::convertToFileData(const QStringList &paths) const
{
    QVector<FileData> result;
    result.reserve(paths.size());

    for (const QString &path : paths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            result.append(FileData::fromFileInfo(fileInfo));
        }
    }

    return result;
}

QVector<FileData> LuceneSearchEngine::getAllFiles(int limit) const
{
    // 对于Lucene++实现，我们可以先做一个空搜索来获取所有文件
    // 这里可以实现缓存以提高性能
    if (m_cachedAllFiles.isEmpty()) {
        QStringList allPaths = performLuceneSearch(m_currentPath, "*", false, false, false);
        m_cachedAllFiles = convertToFileData(allPaths);
    }

    if (limit < 0 || limit >= m_cachedAllFiles.size()) {
        return m_cachedAllFiles;
    }

    return QVector<FileData>(m_cachedAllFiles.constBegin(), m_cachedAllFiles.constBegin() + limit);
}

QVector<FileData> LuceneSearchEngine::searchFiles(const QString &keyword,
                                                bool caseSensitive,
                                                bool fuzzySearch) const
{
    // 这里设置一个标志，防止在执行过程中主线程阻塞
    m_searchCancelled = false;
    
    try {
        QStringList paths = performLuceneSearch(m_currentPath, keyword, false, caseSensitive, fuzzySearch);
        return convertToFileData(paths);
    } catch (...) {
        return QVector<FileData>();
    }
}

void LuceneSearchEngine::cancelSearch()
{
    m_searchCancelled = true;
}

void LuceneSearchEngine::clearCache()
{
    m_cachedAllFiles.clear();
}

QVector<FileData> LuceneSearchEngine::searchFilesBatch(const QString &keyword,
                                                     int offset,
                                                     int limit,
                                                     bool caseSensitive,
                                                     bool fuzzySearch) const
{
    if (keyword.isEmpty() || limit <= 0) {
        return QVector<FileData>();
    }

    try {
        // 获取搜索结果
        QStringList allPaths = performLuceneSearch(m_currentPath, keyword, false, caseSensitive, fuzzySearch);
        
        // 计算分页
        int startIdx = qMin(offset, allPaths.size());
        int endIdx = qMin(offset + limit, allPaths.size());
        
        if (startIdx >= endIdx) {
            return QVector<FileData>();
        }
        
        // 只转换需要的部分
        QStringList batchPaths = allPaths.mid(startIdx, endIdx - startIdx);
        return convertToFileData(batchPaths);
    } catch (...) {
        return QVector<FileData>();
    }
}

int LuceneSearchEngine::getSearchResultCount(const QString &keyword,
                                           bool caseSensitive,
                                           bool fuzzySearch) const
{
    // 确保重置取消标志
    m_searchCancelled = false;
    
    // 获取匹配数量
    QStringList allPaths = performLuceneSearch(m_currentPath, keyword, false, caseSensitive, fuzzySearch);
    return allPaths.size();
}

LuceneSearchEngine::SearchType LuceneSearchEngine::determineSearchType(const QString &keyword, bool fuzzySearch) const
{
    // 如果启用模糊搜索，则直接返回模糊搜索类型
    if (fuzzySearch) {
        return SearchType::Fuzzy;
    }
    
    // 如果包含空格，则为布尔搜索
    if (keyword.contains(' ')) {
        return SearchType::Boolean;
    }

    // 如果包含通配符
    if (keyword.contains('*') || keyword.contains('?')) {
        return SearchType::Wildcard;
    }

    // 默认为简单搜索
    return SearchType::Simple;
}

Lucene::QueryPtr LuceneSearchEngine::buildSearchQuery(const QString &keyword, 
                                                    bool caseSensitive,
                                                    bool fuzzySearch) const
{
    // 星号表示全部匹配
    if (keyword == "*") {
        return newLucene<MatchAllDocsQuery>();
    }

    SearchType searchType = determineSearchType(keyword, fuzzySearch);
    
    // 处理字符串的函数，根据大小写敏感设置决定是否转换为小写
    auto processString = [caseSensitive](const std::wstring& str) -> std::wstring {
        if (caseSensitive) {
            return str;
        } else {
            return StringUtils::toLower(str);
        }
    };

    switch (searchType) {
    case SearchType::Fuzzy: {
        // 模糊搜索实现 - 不使用 FuzzyQuery，而是使用 WildcardQuery 和自定义匹配
        BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
        
        // 设置最大子句数量
        booleanQuery->setMaxClauseCount(1024);
        
        // 分割关键词
        QStringList terms = keyword.split(' ', Qt::SkipEmptyParts);
        
        for (const QString &term : terms) {
            if (term.isEmpty()) continue;
            
            // 创建一个布尔子查询，让一个词可以匹配多种模式
            BooleanQueryPtr termQuery = newLucene<BooleanQuery>();
            
            // 1. 添加精确匹配 (加权最高)
            String exactTerm = processString(StringUtils::toUnicode(term.toStdString()));
            QueryPtr exactQuery = newLucene<TermQuery>(newLucene<Term>(L"file_name", exactTerm));
            // 提高精确匹配的权重
            exactQuery->setBoost(3.0);
            termQuery->add(exactQuery, BooleanClause::SHOULD);
            
            // 2. 添加前缀匹配
            if (term.length() > 2) {
                String prefixTerm = processString(StringUtils::toUnicode(term.toStdString()));
                QueryPtr prefixQuery = newLucene<PrefixQuery>(newLucene<Term>(L"file_name", prefixTerm));
                prefixQuery->setBoost(2.0);
                termQuery->add(prefixQuery, BooleanClause::SHOULD);
            }
            
            // 3. 添加通配符匹配 (允许中间有一个字符不同)
            if (term.length() > 3) {
                for (int i = 1; i < term.length() - 1; i++) {
                    QString fuzzyPattern = term.left(i) + "*" + term.mid(i+1);
                    String wildcardTerm = processString(StringUtils::toUnicode(fuzzyPattern.toStdString()));
                    QueryPtr wildcardQuery = newLucene<WildcardQuery>(newLucene<Term>(L"file_name", wildcardTerm));
                    wildcardQuery->setBoost(1.0);
                    termQuery->add(wildcardQuery, BooleanClause::SHOULD);
                }
            }
            
            // 4. 添加包含匹配 (匹配包含该词的结果)
            String containsTerm = L"*" + processString(StringUtils::toUnicode(term.toStdString())) + L"*";
            QueryPtr containsQuery = newLucene<WildcardQuery>(newLucene<Term>(L"file_name", containsTerm));
            containsQuery->setBoost(1.5);
            termQuery->add(containsQuery, BooleanClause::SHOULD);
            
            // 将该词的所有可能匹配添加到主查询
            booleanQuery->add(termQuery, BooleanClause::MUST);
        }
        
        return booleanQuery;
    }

    case SearchType::Boolean: {
        // 空格分隔的关键词，构建布尔查询
        BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
        
        // 设置最大子句数量，提高性能
        booleanQuery->setMaxClauseCount(1024);
        
        // 分割关键词
        QStringList terms = keyword.split(' ', Qt::SkipEmptyParts);
        
        for (const QString &term : terms) {
            if (!term.isEmpty()) {
                // 为每个关键词创建通配符查询
                String termStr = L"*" + processString(StringUtils::toUnicode(term.toStdString())) + L"*";
                TermPtr termObj = newLucene<Term>(L"file_name", termStr);
                QueryPtr termQuery = newLucene<WildcardQuery>(termObj);

                // 添加到布尔查询(MUST表示AND关系)
                booleanQuery->add(termQuery, BooleanClause::MUST);
            }
        }
        
        return booleanQuery;
    }

    case SearchType::Wildcard:
        // 直接使用用户输入的通配符
        return newLucene<WildcardQuery>(
                newLucene<Term>(L"file_name",
                                processString(StringUtils::toUnicode(keyword.toStdString()))));

    case SearchType::Simple:
    default:
        // 简单查询，加上前后通配符
        String queryString = L"*" + processString(StringUtils::toUnicode(keyword.toStdString())) + L"*";
        return newLucene<WildcardQuery>(newLucene<Term>(L"file_name", queryString));
    }
}

void LuceneSearchEngine::warmupIndex() const
{
    try {
        QString indexDir = getIndexDirectory();
        FSDirectoryPtr directory = FSDirectory::open(StringUtils::toUnicode(indexDir.toStdString()));
        
        if (IndexReader::indexExists(directory)) {
            // 预热索引 - 打开再关闭
            IndexReaderPtr reader = IndexReader::open(directory, true);
            SearcherPtr searcher = newLucene<IndexSearcher>(reader);
            
            // 执行简单查询预热搜索机制
            TermPtr term = newLucene<Term>(L"file_name", L"*");
            QueryPtr query = newLucene<WildcardQuery>(term);
            searcher->search(query, 10);
        }
    } catch (...) {
        // 预热失败不影响主功能
    }
}
