#include "contentsearcher.h"
#include "chineseanalyzer.h"

#include <QDir>
#include <QDateTime>
#include <QRegularExpression>

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
            // 使用搜索引擎风格的内容展示，传递原始关键词
            result.highlightedContent = getSearchEngineStyleContent(content, query, 50, keyword);

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
            // 使用搜索引擎风格的内容展示，传递原始关键词
            result.highlightedContent = getSearchEngineStyleContent(content, booleanQuery, 50, keywords.join(" "));

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

QString ContentSearcher::getSearchEngineStyleContent(const String &content, const QueryPtr &query, 
                                                   int maxLength, const QString &originalKeyword)
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

        // 尝试使用Lucene高亮器
        FormatterPtr formatter = newLucene<SimpleHTMLFormatter>(L"<b style=\"color:red;\">", L"</b>");
        HighlighterScorerPtr scorer = newLucene<QueryScorer>(query);
        HighlighterPtr highlighter = newLucene<Highlighter>(formatter, scorer);

        TokenStreamPtr tokenStream = analyzer->tokenStream(L"contents", newLucene<StringReader>(content));
        Collection<String> fragments = highlighter->getBestFragments(tokenStream, content, 1);

        QString result;
        if (!fragments.empty() && !fragments[0].empty()) {
            // Lucene高亮成功，使用其结果
            result = QString::fromStdWString(fragments[0]);
        } else {
            // Lucene高亮失败，使用自定义高亮方法，传递原始关键词
            result = customHighlight(content, query, contentLength, originalKeyword);
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

// 修改自定义高亮方法，优先使用原始关键词
QString ContentSearcher::customHighlight(const String &content, const QueryPtr &query, 
                                       int maxLength, const QString &originalKeyword)
{
    // 将宽字符串转换为QString以便处理
    QString qContent = QString::fromStdWString(content).simplified();
    
    // 如果有原始关键词，优先使用
    if (!originalKeyword.isEmpty()) {
        int firstMatchPos = qContent.toLower().indexOf(originalKeyword.toLower());
        if (firstMatchPos >= 0) {
            // 找到匹配，使用原始关键词
            return highlightKeywordInContext(qContent, originalKeyword, firstMatchPos, maxLength);
        }
    }
    
    // 如果没有原始关键词或未找到匹配，回退到从查询中提取关键词
    QStringList keywords = extractKeywords(query);
    if (keywords.isEmpty()) {
        // 无法提取关键词，返回内容前段
        int displayLength = std::min(maxLength, static_cast<int>(content.size()));
        return QString::fromStdWString(content.substr(0, displayLength)) + 
               (content.size() > displayLength ? "..." : "");
    }
    
    // 查找第一个关键词匹配
    int firstMatchPos = -1;
    QString matchedKeyword;
    
    for (const QString &keyword : keywords) {
        // 忽略大小写搜索
        int pos = qContent.toLower().indexOf(keyword.toLower());
        if (pos >= 0 && (firstMatchPos == -1 || pos < firstMatchPos)) {
            firstMatchPos = pos;
            matchedKeyword = keyword;
        }
    }
    
    if (firstMatchPos == -1) {
        // 没有找到匹配项，返回内容前段
        int displayLength = std::min(maxLength, static_cast<int>(qContent.length()));
        return qContent.left(displayLength) + (qContent.length() > displayLength ? "..." : "");
    }
    
    return highlightKeywordInContext(qContent, matchedKeyword, firstMatchPos, maxLength);
}

// 新增上下文高亮辅助方法
QString ContentSearcher::highlightKeywordInContext(const QString &content, const QString &keyword, 
                                                 int matchPos, int maxLength)
{
    // 计算上下文范围
    int contextStart = std::max(0, matchPos - maxLength / 2);
    int keywordLength = keyword.length();
    int contextEnd = std::min(static_cast<int>(content.length()),
                             matchPos + keywordLength + maxLength / 2);
    
    // 调整以避免截断单词
    if (contextStart > 0) {
        // 向前找到空格或标点
        int i = contextStart;
        while (i > 0 && !content[i].isSpace() && !content[i].isPunct()) {
            i--;
        }
        if (i > 0) contextStart = i + 1;
    }
    
    if (contextEnd < content.length()) {
        // 向后找到空格或标点
        int i = contextEnd;
        while (i < content.length() && !content[i].isSpace() && !content[i].isPunct()) {
            i++;
        }
        if (i < content.length()) contextEnd = i;
    }
    
    // 提取上下文并添加省略号
    QString result = content.mid(contextStart, contextEnd - contextStart);
    if (contextStart > 0) result = "..." + result;
    if (contextEnd < content.length()) result = result + "...";
    
    // 高亮关键词
    QString lcResult = result.toLower();
    QString lcKeyword = keyword.toLower();
    int pos = 0;
    
    while ((pos = lcResult.indexOf(lcKeyword, pos)) != -1) {
        // 检查是否需要整词匹配
        bool isWordStart = (pos == 0 || !lcResult[pos - 1].isLetterOrNumber());
        bool isWordEnd = (pos + lcKeyword.length() >= lcResult.length() || 
                         !lcResult[pos + lcKeyword.length()].isLetterOrNumber());
        
        // 对于英文单词，需要完整匹配，对于中文或特殊情况，可以松一些
        bool shouldHighlight = true;
        if (keyword.at(0).isLetter()) {
            // 英文单词需要整词匹配
            shouldHighlight = isWordStart && isWordEnd;
        }
        
        if (shouldHighlight) {
            // 获取原始大小写的关键词
            QString originalCaseKeyword = result.mid(pos, lcKeyword.length());
            // 替换为高亮版本
            result.replace(pos, lcKeyword.length(),
                          "<b style=\"color:red;\">" + originalCaseKeyword + "</b>");
            // 更新低版本结果和位置
            lcResult = result.toLower();
            pos += 29 + lcKeyword.length();
        } else {
            pos += lcKeyword.length();
        }
    }
    
    return result;
}

QStringList ContentSearcher::extractKeywords(const QueryPtr &query)
{
    QStringList keywords;

    try {
        // 处理布尔查询
        BooleanQueryPtr boolQuery = boost::dynamic_pointer_cast<BooleanQuery>(query);
        if (boolQuery) {
            // 使用getClauses()方法而不是直接访问保护成员
            Collection<BooleanClausePtr> clauses = boolQuery->getClauses();
            for (int i = 0; i < clauses.size(); i++) {
                BooleanClausePtr clause = clauses[i];
                keywords.append(extractKeywords(clause->getQuery()));
            }
            return keywords;
        }

        // 处理词条查询
        TermQueryPtr termQuery = boost::dynamic_pointer_cast<TermQuery>(query);
        if (termQuery) {
            keywords.append(QString::fromStdWString(termQuery->getTerm()->text()));
            return keywords;
        }

        // 处理短语查询
        PhraseQueryPtr phraseQuery = boost::dynamic_pointer_cast<PhraseQuery>(query);
        if (phraseQuery) {
            Collection<TermPtr> terms = phraseQuery->getTerms();
            QStringList phraseWords;
            for (int i = 0; i < terms.size(); i++) {
                phraseWords.append(QString::fromStdWString(terms[i]->text()));
            }
            keywords.append(phraseWords.join(" "));
            return keywords;
        }

        // 处理通配符查询
        WildcardQueryPtr wildcardQuery = boost::dynamic_pointer_cast<WildcardQuery>(query);
        if (wildcardQuery) {
            // 使用去掉通配符的字符串
            QString termText = QString::fromStdWString(wildcardQuery->getTerm()->text());
            termText.remove('*').remove('?');
            if (!termText.isEmpty()) {
                keywords.append(termText);
            }
            return keywords;
        }

        // 如果是其他类型的查询，尝试获取查询文本
        String queryText = query->toString();
        QString qQueryText = QString::fromStdWString(queryText);

        // 手动解析查询文本
        qQueryText.remove(QRegularExpression(R"(\w+:)"));   // 移除字段名（如 "title:apple" → "apple"）
        qQueryText.remove(QRegularExpression(R"([$$\+\-\&\|\!\{\}$$$$\^\~\*\?\\:])"));   // 移除特殊字符
        qQueryText = qQueryText.trimmed();   // 去除前后空格

        if (!qQueryText.isEmpty()) {
            keywords.append(qQueryText);
        }
    } catch (const std::exception &e) {
        qWarning() << "Error extracting keywords from query:" << e.what();
    }

    return keywords;
}
