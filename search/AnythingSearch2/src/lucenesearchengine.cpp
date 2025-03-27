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

QStringList LuceneSearchEngine::performLuceneSearch(const QString &originPath, const QString &key, bool nrt) const
{
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

        // 使用正确的方法调用
        Lucene::QueryPtr query = buildSearchQuery(keywords);

        // 执行搜索
        TopDocsPtr search_results = searcher->search(query, max_results);

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
        QStringList allPaths = performLuceneSearch(m_currentPath, "*", false);
        m_cachedAllFiles = convertToFileData(allPaths);
    }

    if (limit < 0 || limit >= m_cachedAllFiles.size()) {
        return m_cachedAllFiles;
    }

    return QVector<FileData>(m_cachedAllFiles.constBegin(), m_cachedAllFiles.constBegin() + limit);
}

QVector<FileData> LuceneSearchEngine::searchFiles(const QString &keyword) const
{
    m_searchCancelled = false;
    if (keyword.isEmpty()) {
        return getAllFiles(1000);   // 限制返回数量以保证性能
    }

    QStringList paths = performLuceneSearch(m_currentPath, keyword, false);
    return convertToFileData(paths);
}

void LuceneSearchEngine::cancelSearch()
{
    m_searchCancelled = true;
}

void LuceneSearchEngine::clearCache()
{
    m_cachedAllFiles.clear();
}

QVector<FileData> LuceneSearchEngine::searchFilesBatch(const QString &keyword, int offset, int limit) const
{
    // 确保重置取消标志
    m_searchCancelled = false;
    
    // 获取所有匹配路径
    QStringList allPaths = performLuceneSearch(m_currentPath, keyword, false);

    // 计算分页
    int startIdx = qMin(offset, allPaths.size());
    int endIdx = qMin(offset + limit, allPaths.size());

    // 提取当前页的路径
    QStringList batchPaths;
    for (int i = startIdx; i < endIdx; ++i) {
        batchPaths.append(allPaths.at(i));
    }

    // 转换为FileData
    return convertToFileData(batchPaths);
}

int LuceneSearchEngine::getSearchResultCount(const QString &keyword) const
{
    // 确保重置取消标志
    m_searchCancelled = false;
    
    // 获取匹配数量
    QStringList allPaths = performLuceneSearch(m_currentPath, keyword, false);
    return allPaths.size();
}

LuceneSearchEngine::SearchType LuceneSearchEngine::determineSearchType(const QString &keyword) const
{
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

Lucene::QueryPtr LuceneSearchEngine::buildSearchQuery(const QString &keyword) const
{
    // 星号表示全部匹配
    if (keyword == "*") {
        return newLucene<MatchAllDocsQuery>();
    }

    SearchType searchType = determineSearchType(keyword);

    switch (searchType) {
    case SearchType::Boolean: {
        // 空格分隔的关键词，构建布尔查询
        BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
        
        // 分割关键词
        QStringList terms = keyword.split(' ', Qt::SkipEmptyParts);
        
        for (const QString &term : terms) {
            if (!term.isEmpty()) {
                // 为每个关键词创建通配符查询
                String termStr = L"*" + StringUtils::toLower(StringUtils::toUnicode(term.toStdString())) + L"*";
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
                                StringUtils::toLower(StringUtils::toUnicode(keyword.toStdString()))));

    case SearchType::Simple:
    default:
        // 简单查询，加上前后通配符
        String queryString = L"*" + StringUtils::toLower(StringUtils::toUnicode(keyword.toStdString())) + L"*";
        return newLucene<WildcardQuery>(newLucene<Term>(L"file_name", queryString));
    }
}
