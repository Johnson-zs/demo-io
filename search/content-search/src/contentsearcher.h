#ifndef CONTENTSEARCHER_H
#define CONTENTSEARCHER_H

#include <QString>
#include <QList>
#include <QDebug>

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

    QList<SearchResult> search(const QString &keyword, int maxResults = 10, const QString &searchPath = QString());

private:
    QString getHighlightedContent(const DocumentPtr &doc, const QueryPtr &query);

    IndexReaderPtr reader;
    IndexSearcherPtr searcher;
    AnalyzerPtr analyzer;
};

#endif   // CONTENTSEARCHER_H
