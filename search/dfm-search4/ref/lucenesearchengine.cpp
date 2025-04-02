#include "lucenesearchengine.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <unistd.h>
#include <QRegularExpression>

using namespace Lucene;

//--------------------------------------------------------------------
// QueryBuilder 实现
//--------------------------------------------------------------------

QueryBuilder::QueryBuilder()
{
}

Lucene::String QueryBuilder::processString(const QString &str, bool caseSensitive) const
{
    String unicodeStr = StringUtils::toUnicode(str.toStdString());
    if (caseSensitive) {
        return unicodeStr;
    } else {
        StringUtils::toLower(unicodeStr);
        return unicodeStr;
    }
}

Lucene::QueryPtr QueryBuilder::buildTypeQuery(const QString &typeList) const
{
    // 将类型字符串分割成列表
    QStringList types = typeList.split(',', Qt::SkipEmptyParts);

    // 构建文件类型查询
    BooleanQueryPtr typeQuery = newLucene<BooleanQuery>();

    for (const QString &type : types) {
        QString cleanType = type.trimmed().toLower();

        if (!cleanType.isEmpty()) {
            // 创建类型词项查询
            QueryPtr termQuery = newLucene<TermQuery>(
                    newLucene<Term>(L"file_type",
                                    StringUtils::toUnicode(cleanType.toStdString())));

            // 添加到类型查询 (OR关系)
            typeQuery->add(termQuery, BooleanClause::SHOULD);
        }
    }

    return typeQuery;
}

Lucene::QueryPtr QueryBuilder::buildPinyinQuery(const QString &pinyinList) const
{
    // 将拼音字符串分割成列表
    QStringList pinyins = pinyinList.split(',', Qt::SkipEmptyParts);

    // 构建拼音查询
    BooleanQueryPtr pinyinQuery = newLucene<BooleanQuery>();

    for (const QString &pinyin : pinyins) {
        QString cleanPinyin = pinyin.trimmed().toLower();

        if (!cleanPinyin.isEmpty()) {
            // 创建拼音词项查询
            QueryPtr termQuery = newLucene<WildcardQuery>(
                    newLucene<Term>(L"pinyin",
                                    StringUtils::toUnicode(QString("*%1*").arg(cleanPinyin).toStdString())));

            // 添加到拼音查询 (OR关系)
            pinyinQuery->add(termQuery, BooleanClause::SHOULD);
        }
    }

    return pinyinQuery;
}

Lucene::QueryPtr QueryBuilder::buildFuzzyQuery(const QString &keyword, bool caseSensitive) const
{
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
        String exactTerm = processString(term, caseSensitive);
        QueryPtr exactQuery = newLucene<TermQuery>(newLucene<Term>(L"file_name", exactTerm));
        // 提高精确匹配的权重
        exactQuery->setBoost(3.0);
        termQuery->add(exactQuery, BooleanClause::SHOULD);

        // 2. 添加前缀匹配
        if (term.length() > 2) {
            String prefixTerm = processString(term, caseSensitive);
            QueryPtr prefixQuery = newLucene<PrefixQuery>(newLucene<Term>(L"file_name", prefixTerm));
            prefixQuery->setBoost(2.0);
            termQuery->add(prefixQuery, BooleanClause::SHOULD);
        }

        // 3. 添加通配符匹配 (允许中间有一个字符不同)
        if (term.length() > 3) {
            for (int i = 1; i < term.length() - 1; i++) {
                QString fuzzyPattern = term.left(i) + "*" + term.mid(i + 1);
                String wildcardTerm = processString(fuzzyPattern, caseSensitive);
                QueryPtr wildcardQuery = newLucene<WildcardQuery>(newLucene<Term>(L"file_name", wildcardTerm));
                wildcardQuery->setBoost(1.0);
                termQuery->add(wildcardQuery, BooleanClause::SHOULD);
            }
        }

        // 4. 添加包含匹配 (匹配包含该词的结果)
        String containsTerm = L"*" + processString(term, caseSensitive) + L"*";
        QueryPtr containsQuery = newLucene<WildcardQuery>(newLucene<Term>(L"file_name", containsTerm));
        containsQuery->setBoost(1.5);
        termQuery->add(containsQuery, BooleanClause::SHOULD);

        // 将该词的所有可能匹配添加到主查询
        booleanQuery->add(termQuery, BooleanClause::MUST);
    }

    return booleanQuery;
}

Lucene::QueryPtr QueryBuilder::buildBooleanQuery(const QString &keyword, bool caseSensitive) const
{
    // 空格分隔的关键词，构建布尔查询
    BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();

    // 设置最大子句数量，提高性能
    booleanQuery->setMaxClauseCount(1024);

    // 分割关键词
    QStringList terms = keyword.split(' ', Qt::SkipEmptyParts);

    for (const QString &term : terms) {
        if (!term.isEmpty()) {
            // 为每个关键词创建通配符查询
            String termStr = L"*" + processString(term, caseSensitive) + L"*";
            TermPtr termObj = newLucene<Term>(L"file_name", termStr);
            QueryPtr termQuery = newLucene<WildcardQuery>(termObj);

            // 添加到布尔查询(MUST表示AND关系)
            booleanQuery->add(termQuery, BooleanClause::MUST);
        }
    }

    return booleanQuery;
}

Lucene::QueryPtr QueryBuilder::buildWildcardQuery(const QString &keyword, bool caseSensitive) const
{
    // 直接使用用户输入的通配符
    return newLucene<WildcardQuery>(
            newLucene<Term>(L"file_name",
                            processString(keyword, caseSensitive)));
}

Lucene::QueryPtr QueryBuilder::buildSimpleQuery(const QString &keyword, bool caseSensitive) const
{
    // 简单查询，加上前后通配符
    String queryString = L"*" + processString(keyword, caseSensitive) + L"*";
    return newLucene<WildcardQuery>(newLucene<Term>(L"file_name", queryString));
}

//--------------------------------------------------------------------
// SearchCache 实现
//--------------------------------------------------------------------

SearchCache::SearchCache()
    : m_lastCaseSensitive(false), m_lastFuzzySearch(false)
{
}

QStringList SearchCache::getCachedResults(const QString &keyword, bool caseSensitive, bool fuzzySearch) const
{
    if (isCacheHit(keyword, caseSensitive, fuzzySearch)) {
        return m_lastResults;
    }
    return QStringList();
}

void SearchCache::storeResults(const QString &keyword, const QStringList &results, bool caseSensitive, bool fuzzySearch)
{
    m_lastKeyword = keyword;
    m_lastResults = results;
    m_lastCaseSensitive = caseSensitive;
    m_lastFuzzySearch = fuzzySearch;
}

void SearchCache::clearCache()
{
    m_lastKeyword.clear();
    m_lastResults.clear();
}

bool SearchCache::isCacheHit(const QString &keyword, bool caseSensitive, bool fuzzySearch) const
{
    return !m_lastKeyword.isEmpty() && keyword == m_lastKeyword && !m_lastResults.isEmpty() && caseSensitive == m_lastCaseSensitive && fuzzySearch == m_lastFuzzySearch;
}

//--------------------------------------------------------------------
// IndexManager 实现
//--------------------------------------------------------------------

IndexManager::IndexManager()
{
}

FSDirectoryPtr IndexManager::getIndexDirectory(const QString &indexPath) const
{
    if (m_cachedDirectory && m_cachedIndexPath == indexPath) {
        return m_cachedDirectory;
    }

    m_cachedIndexPath = indexPath;
    m_cachedDirectory = FSDirectory::open(StringUtils::toUnicode(indexPath.toStdString()));
    return m_cachedDirectory;
}

IndexReaderPtr IndexManager::getIndexReader(FSDirectoryPtr directory) const
{
    if (m_cachedReader && m_cachedDirectory == directory) {
        return m_cachedReader;
    }

    if (IndexReader::indexExists(directory)) {
        m_cachedReader = IndexReader::open(directory, true);
        return m_cachedReader;
    }

    return nullptr;
}

SearcherPtr IndexManager::getSearcher(IndexReaderPtr reader) const
{
    if (m_cachedSearcher && m_cachedReader == reader) {
        return m_cachedSearcher;
    }

    m_cachedSearcher = newLucene<IndexSearcher>(reader);
    return m_cachedSearcher;
}

void IndexManager::warmupIndex(const QString &indexPath) const
{
    try {
        FSDirectoryPtr directory = getIndexDirectory(indexPath);

        if (IndexReader::indexExists(directory)) {
            IndexReaderPtr reader = getIndexReader(directory);
            SearcherPtr searcher = getSearcher(reader);

            // 执行简单查询预热
            TermPtr term = newLucene<Term>(L"file_name", L"*");
            QueryPtr query = newLucene<WildcardQuery>(term);
            searcher->search(query, 10);

            // 预取一些文档
            if (reader && reader->numDocs() > 0) {
                // 预热一些文档
                for (int i = 0; i < std::min(10, reader->numDocs()); i++) {
                    searcher->doc(i);
                }
            }
        }
    } catch (...) {
        // 预热失败不影响主功能
    }
}

//--------------------------------------------------------------------
// LuceneSearchEngine 实现
//--------------------------------------------------------------------

LuceneSearchEngine::LuceneSearchEngine()
    : m_searchCancelled(false),
      m_lastCaseSensitive(false),
      m_lastFuzzySearch(false)
{

    // 初始化组件
    m_queryBuilder = std::make_unique<QueryBuilder>();
    m_searchCache = std::make_unique<SearchCache>();
    m_indexManager = std::make_unique<IndexManager>();
}

LuceneSearchEngine::~LuceneSearchEngine()
{
}

void LuceneSearchEngine::updateSearchPath(const QString &path)
{
    m_currentPath = path;
    m_cachedAllFiles.clear();
    m_lastKeyword.clear();
    m_lastSearchResults.clear();
}

void LuceneSearchEngine::cancelSearch()
{
    m_searchCancelled = true;
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

LuceneSearchEngine::SearchType LuceneSearchEngine::determineSearchType(const QString &keyword, bool fuzzySearch) const
{
    // 有通配符的搜索
    if (keyword.contains('*') || keyword.contains('?')) {
        return SearchType::Wildcard;
    }

    // 空格分隔的多个词
    if (keyword.contains(' ')) {
        return SearchType::Boolean;
    }

    // 启用模糊搜索
    if (fuzzySearch) {
        return SearchType::Fuzzy;
    }

    // 默认简单搜索
    return SearchType::Simple;
}

QVector<FileData> LuceneSearchEngine::searchFiles(const QString &keyword,
                                                  bool caseSensitive,
                                                  bool fuzzySearch) const
{
    // 重置取消标志
    m_searchCancelled = false;

    try {
        QStringList paths = performLuceneSearch(m_currentPath, keyword, false, caseSensitive, fuzzySearch);
        return convertToFileData(paths);
    } catch (...) {
        return QVector<FileData>();
    }
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

QVector<FileData> LuceneSearchEngine::getAllFiles(int limit) const
{
    if (!m_cachedAllFiles.isEmpty()) {
        if (limit < 0 || limit >= m_cachedAllFiles.size()) {
            return convertToFileData(m_cachedAllFiles);
        } else {
            return convertToFileData(m_cachedAllFiles.mid(0, limit));
        }
    }

    try {
        // 使用 performLuceneSearch 获取所有文件
        QStringList results = performLuceneSearch(m_currentPath, "*", true, false, false);

        // 缓存结果
        m_cachedAllFiles = results;

        // 转换结果
        if (limit < 0 || limit >= results.size()) {
            return convertToFileData(results);
        } else {
            return convertToFileData(results.mid(0, limit));
        }
    } catch (...) {
        return QVector<FileData>();
    }
}

QStringList LuceneSearchEngine::performLuceneSearch(const QString &originPath,
                                                    const QString &key,
                                                    bool nrt,
                                                    bool caseSensitive,
                                                    bool fuzzySearch) const
{
    // 使用缓存
    if (!nrt && m_searchCache->isCacheHit(key, caseSensitive, fuzzySearch)) {
        return m_searchCache->getCachedResults(key, caseSensitive, fuzzySearch);
    }

    // 重置取消标志
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
        FSDirectoryPtr directory = m_indexManager->getIndexDirectory(indexDir);

        // 检查索引是否存在
        if (!IndexReader::indexExists(directory)) {
            qWarning() << "索引不存在:" << indexDir;
            return QStringList();
        }

        // 获取索引读取器
        IndexReaderPtr reader = m_indexManager->getIndexReader(directory);
        if (!reader || reader->numDocs() == 0) {
            qWarning() << "索引为空，没有文档";
            return QStringList();
        }

        // 获取搜索器
        SearcherPtr searcher = m_indexManager->getSearcher(reader);
        int32_t max_results = reader->numDocs();

        // 构建查询
        Lucene::QueryPtr query = buildSearchQuery(keywords, caseSensitive, fuzzySearch);

        // 创建过滤器，仅搜索指定路径下的文件
        FilterPtr pathFilter = newLucene<PrefixFilter>(
                newLucene<Term>(L"full_path", StringUtils::toUnicode(path.toStdString())));

        // 执行搜索 - 不使用过滤器以提高性能
        TopDocsPtr search_results = searcher->search(query, max_results);

        // 处理结果
        QStringList results;
        results.reserve(search_results->scoreDocs.size());
        QStringList dirs, files;
        dirs.reserve(search_results->scoreDocs.size() / 2);
        files.reserve(search_results->scoreDocs.size() / 2);

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
                dirs.append(std::move(result));
            } else {
                files.append(std::move(result));
            }
        }

        // 合并结果（目录在前，文件在后）
        results = std::move(dirs) + std::move(files);

        // 存储结果到缓存
        if (!nrt) {
            m_searchCache->storeResults(key, results, caseSensitive, fuzzySearch);
        }

        return results;
    } catch (const LuceneException &e) {
        qWarning() << "Lucene搜索异常:" << QString::fromStdWString(e.getError());
        return {};
    } catch (const std::exception &e) {
        qWarning() << "标准异常:" << e.what();
        return {};
    } catch (...) {
        qWarning() << "未知异常";
        return {};
    }
}

Lucene::QueryPtr LuceneSearchEngine::buildSearchQuery(const QString &keyword,
                                                      bool caseSensitive,
                                                      bool fuzzySearch) const
{
    // 检查是否包含拼音搜索语法
    QRegularExpression pinyinRegex("py:([\\w,]+)");
    QRegularExpressionMatch pinyinMatch = pinyinRegex.match(keyword);

    if (pinyinMatch.hasMatch()) {
        // 提取拼音列表和其他关键词
        QString pinyinList = pinyinMatch.captured(1);
        QString remainingKeyword = keyword;
        remainingKeyword = remainingKeyword.replace(pinyinMatch.captured(0), "").trimmed();

        // 构建拼音查询
        BooleanQueryPtr query = newLucene<BooleanQuery>();

        // 添加拼音条件
        if (!pinyinList.isEmpty()) {
            QueryPtr pinyinQuery = m_queryBuilder->buildPinyinQuery(pinyinList);
            query->add(pinyinQuery, BooleanClause::MUST);
        }

        // 如果还有其他关键词，添加文件名条件
        if (!remainingKeyword.isEmpty()) {
            QueryPtr nameQuery = buildSearchQuery(remainingKeyword, caseSensitive, fuzzySearch);
            query->add(nameQuery, BooleanClause::MUST);
        }

        return query;
    }

    // 检查是否包含文件类型搜索语法
    QRegularExpression typeRegex("type:([\\w,]+)");
    QRegularExpressionMatch match = typeRegex.match(keyword);

    if (match.hasMatch()) {
        // 提取文件类型列表和其他关键词
        QString typeList = match.captured(1);
        QString remainingKeyword = keyword;
        remainingKeyword = remainingKeyword.replace(match.captured(0), "").trimmed();

        // 构建文件类型查询
        BooleanQueryPtr query = newLucene<BooleanQuery>();

        // 添加文件类型条件
        if (!typeList.isEmpty()) {
            QueryPtr typeQuery = m_queryBuilder->buildTypeQuery(typeList);
            query->add(typeQuery, BooleanClause::MUST);
        }

        // 如果还有其他关键词，添加文件名条件
        if (!remainingKeyword.isEmpty()) {
            QueryPtr nameQuery = buildSearchQuery(remainingKeyword, caseSensitive, fuzzySearch);
            query->add(nameQuery, BooleanClause::MUST);
        }

        return query;
    }

    // 确定搜索类型
    SearchType searchType = determineSearchType(keyword, fuzzySearch);

    // 根据搜索类型使用合适的查询策略
    switch (searchType) {
    case SearchType::Fuzzy:
        return m_queryBuilder->buildFuzzyQuery(keyword, caseSensitive);
    case SearchType::Boolean:
        return m_queryBuilder->buildBooleanQuery(keyword, caseSensitive);
    case SearchType::Wildcard:
        return m_queryBuilder->buildWildcardQuery(keyword, caseSensitive);
    case SearchType::Simple:
    default:
        return m_queryBuilder->buildSimpleQuery(keyword, caseSensitive);
    }
}

void LuceneSearchEngine::warmupIndex() const
{
    m_indexManager->warmupIndex(getIndexDirectory());
}

void LuceneSearchEngine::clearCache()
{
    m_lastKeyword.clear();
    m_lastSearchResults.clear();
    m_cachedAllFiles.clear();
    if (m_searchCache) {
        m_searchCache->clearCache();
    }
}

int LuceneSearchEngine::getSearchResultCount(const QString &keyword, bool caseSensitive, bool fuzzySearch) const
{
    // 利用现有方法获取计数
    if (keyword.isEmpty()) {
        return 0;
    }

    try {
        // 使用现有搜索方法但只返回数量
        QStringList results = performLuceneSearch(m_currentPath, keyword, false, caseSensitive, fuzzySearch);
        return results.size();
    } catch (...) {
        return 0;
    }
}
