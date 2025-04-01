#include "contentsearcher.h"
#include "chineseanalyzer.h"

#include <QDir>
#include <QDateTime>

#include <Highlighter.h>
#include <SimpleHTMLFormatter.h>
#include <SimpleFragmenter.h>
#include <QueryScorer.h>
#include <QueryWrapperFilter.h>

ContentSearcher::ContentSearcher(const QString &indexPath)
{
    try {
        // 打开索引
        reader = IndexReader::open(FSDirectory::open(indexPath.toStdWString()), true);
        searcher = newLucene<IndexSearcher>(reader);
        analyzer = newLucene<ChineseAnalyzer>();
    } catch (const LuceneException &e) {
        qCritical() << "Failed to initialize searcher:" << QString::fromStdWString(e.getError());
        throw;
    }
}

ContentSearcher::~ContentSearcher()
{
    // Resources will be freed by smart pointers
}

QList<SearchResult> ContentSearcher::search(const QString &keyword, int maxResults, const QString &searchPath)
{
    QList<SearchResult> results;

    try {
        // 创建查询解析器
        QueryParserPtr parser = newLucene<QueryParser>(
                LuceneVersion::LUCENE_CURRENT,
                L"contents",
                analyzer);

        parser->setAllowLeadingWildcard(true);

        // 解析查询
        QueryPtr query = parser->parse(keyword.toStdWString());
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));

        // 执行带过滤器的搜索
        TopDocsPtr topDocs = searcher->search(query, filter, maxResults);
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;

        // 处理搜索结果
        for (int32_t i = 0; i < scoreDocs.size(); ++i) {
            ScoreDocPtr scoreDoc = scoreDocs[i];
            DocumentPtr doc = searcher->doc(scoreDoc->doc);

            SearchResult result;
            result.path = QString::fromStdWString(doc->get(L"path"));
            result.modifiedTime = QString::fromStdWString(doc->get(L"modified"));

            // 获取文档内容
            String content = doc->get(L"contents");
            // 使用搜索引擎风格的内容展示
            result.highlightedContent = getSearchEngineStyleContent(content, query);

            results.append(result);
        }
    } catch (const LuceneException &e) {
        qCritical() << "Search failed:" << QString::fromStdWString(e.getError());
    }

    return results;
}

QList<SearchResult> ContentSearcher::booleanAndSearch(const QStringList &keywords, int maxResults, const QString &searchPath)
{
    QList<SearchResult> results;

    if (keywords.isEmpty()) {
        return results;
    }

    try {
        // 创建布尔查询
        BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();

        // 创建查询解析器
        QueryParserPtr parser = newLucene<QueryParser>(
                LuceneVersion::LUCENE_CURRENT,
                L"contents",
                analyzer);

        parser->setAllowLeadingWildcard(true);

        // 添加所有关键词为MUST条件
        for (const QString &keyword : keywords) {
            QueryPtr termQuery = parser->parse(keyword.toStdWString());
            booleanQuery->add(termQuery, BooleanClause::MUST);
        }

        // 创建路径过滤器
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));

        // 执行带过滤器的搜索
        TopDocsPtr topDocs = searcher->search(booleanQuery, filter, maxResults);
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;

        // 处理搜索结果
        for (int32_t i = 0; i < scoreDocs.size(); ++i) {
            ScoreDocPtr scoreDoc = scoreDocs[i];
            DocumentPtr doc = searcher->doc(scoreDoc->doc);

            SearchResult result;
            result.path = QString::fromStdWString(doc->get(L"path"));
            result.modifiedTime = QString::fromStdWString(doc->get(L"modified"));

            // 获取文档内容
            String content = doc->get(L"contents");
            // 使用搜索引擎风格的内容展示
            result.highlightedContent = getSearchEngineStyleContent(content, booleanQuery);

            results.append(result);
        }
    } catch (const LuceneException &e) {
        qCritical() << "Boolean search failed:" << QString::fromStdWString(e.getError());
    }

    return results;
}

QString ContentSearcher::getHighlightedContent(const DocumentPtr &doc, const QueryPtr &query)
{
    try {
        // 获取文档内容
        String content = doc->get(L"contents");
        if (content.empty()) {
            return QStringLiteral("(No content available)");
        }

        // 创建高亮器
        FormatterPtr formatter = newLucene<SimpleHTMLFormatter>(L"<b style=\"color:red;\">", L"</b>");
        HighlighterScorerPtr scorer = newLucene<QueryScorer>(query);
        HighlighterPtr highlighter = newLucene<Highlighter>(formatter, scorer);

        // 设置文本分段器
        FragmenterPtr fragmenter = newLucene<SimpleFragmenter>(150);
        highlighter->setTextFragmenter(fragmenter);

        // 获取高亮片段
        TokenStreamPtr tokenStream = analyzer->tokenStream(L"contents", newLucene<StringReader>(content));
        Collection<String> fragments = highlighter->getBestFragments(tokenStream, content, 3);

        // 合并片段
        QString result;
        for (int32_t i = 0; i < fragments.size(); ++i) {
            if (i > 0) {
                result += QLatin1String("...<br>");
            }
            result += QString::fromStdWString(fragments[i]);
        }

        // 如果没有高亮片段，显示部分原始内容
        if (result.isEmpty()) {
            // 显示前150个字符
            int maxChars = std::min(150, (int)content.size());
            result = QString::fromStdWString(content.substr(0, maxChars));
            if (content.size() > maxChars) {
                result += QLatin1String("...");
            }
        }

        // 处理连续的高亮标签，将它们合并
        result = mergeAdjacentHighlightTags(result);

        return result;
    } catch (const LuceneException &e) {
        qCritical() << "Highlighting failed:" << QString::fromStdWString(e.getError());
        return QStringLiteral("(Error highlighting content)");
    }
}

QString ContentSearcher::getSearchEngineStyleContent(const String &content, const QueryPtr &query, int maxLength)
{
    try {
        if (content.empty()) {
            return QStringLiteral("(No content available)");
        }

        // 确定内容显示长度
        int contentLength = determineContentLength(content);
        if (contentLength <= 0) {
            contentLength = maxLength;
        }

        // 创建高亮器
        FormatterPtr formatter = newLucene<SimpleHTMLFormatter>(L"<b style=\"color:red;\">", L"</b>");
        HighlighterScorerPtr scorer = newLucene<QueryScorer>(query);
        HighlighterPtr highlighter = newLucene<Highlighter>(formatter, scorer);

        // 提取包含关键词的最相关段落
        TokenStreamPtr tokenStream = analyzer->tokenStream(L"contents", newLucene<StringReader>(content));
        Collection<String> fragments = highlighter->getBestFragments(tokenStream, content, 1);   // 只取最相关的一段

        QString result;
        if (!fragments.empty()) {
            // 有高亮内容，直接使用
            result = QString::fromStdWString(fragments[0]);
        } else {
            // 无高亮内容，取开头一段
            int displayLength = std::min(contentLength, (int)content.size());
            result = QString::fromStdWString(content.substr(0, displayLength));
            if (content.size() > displayLength) {
                result += QLatin1String("...");
            }
        }

        // 处理连续的高亮标签
        result = mergeAdjacentHighlightTags(result);

        return result.simplified();
    } catch (const LuceneException &e) {
        qCritical() << "Search engine style content generation failed:" << QString::fromStdWString(e.getError());
        return QStringLiteral("(Error generating content preview)");
    }
}

int ContentSearcher::determineContentLength(const String &content)
{
    // 检查是否有换行符
    size_t newLinePos = content.find(L'\n');
    if (newLinePos != String::npos && newLinePos > 0) {
        // 如果有换行符，以换行符位置为界
        return static_cast<int>(newLinePos);
    }

    // 无换行符的文档(如PDF)，返回50
    return 50;
}

QString ContentSearcher::mergeAdjacentHighlightTags(const QString &text)
{
    // 使用正则表达式搜索和替换相邻的高亮标签
    QString result = text;

    // 替换模式：</b><b style="color:red;"> 将被删除，从而合并相邻的标签
    static const QString pattern = QLatin1String("</b><b style=\"color:red;\">");
    static const QString replacement = QLatin1String("");

    // 循环替换直到不再有变化（处理连续多个标签的情况）
    QString previousResult;
    do {
        previousResult = result;
        result = result.replace(pattern, replacement);
    } while (result != previousResult);

    return result;
}
