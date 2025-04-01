#ifndef CONTENTSEARCHER_H
#define CONTENTSEARCHER_H

#include <QString>
#include <QList>
#include <QDebug>
#include <QDir>
#include <QStringList>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/QueryParser.h>
#include <lucene++/BooleanQuery.h>

using namespace Lucene;

struct SearchResult
{
    QString path;
    QString modifiedTime;
    QString highlightedContent;
};

class ContentSearcher
{
public:
    ContentSearcher(const QString &indexPath);
    ~ContentSearcher();

    // 通用搜索函数
    QList<SearchResult> search(const QString &keyword, int maxResults = 100000, const QString &searchPath = QDir::homePath());
    
    // 布尔"与"搜索函数
    QList<SearchResult> booleanAndSearch(const QStringList &keywords, int maxResults = 100000, const QString &searchPath = QDir::homePath());

private:
    QString getHighlightedContent(const DocumentPtr &doc, const QueryPtr &query);
    
    // 搜索引擎风格的内容展示
    QString getSearchEngineStyleContent(const String &content, const QueryPtr &query, int maxLength = 50);
    
    // 合并相邻的高亮标签
    QString mergeAdjacentHighlightTags(const QString &text);
    
    // 根据文档特性确定截断长度
    int determineContentLength(const String &content);

    IndexReaderPtr reader;
    IndexSearcherPtr searcher;
    AnalyzerPtr analyzer;
};

#endif   // CONTENTSEARCHER_H
