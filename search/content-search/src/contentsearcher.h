#ifndef CONTENTSEARCHER_H
#define CONTENTSEARCHER_H

#include <QString>
#include <QList>
#include <QDebug>
#include <QDir>

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

    QList<SearchResult> search(const QString &keyword, int maxResults = 100000, const QString &searchPath = QDir::homePath());

private:
    QString getHighlightedContent(const DocumentPtr &doc, const QueryPtr &query);

    // 合并相邻的高亮标签
    QString mergeAdjacentHighlightTags(const QString &text);

    IndexReaderPtr reader;
    IndexSearcherPtr searcher;
    AnalyzerPtr analyzer;
};

#endif   // CONTENTSEARCHER_H
