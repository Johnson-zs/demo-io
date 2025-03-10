#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QString>
#include <QStringList>
#include <lucene++/LuceneHeaders.h>

class SearchEngine
{
public:
    SearchEngine();
    ~SearchEngine();

    void createIndex(const QStringList &files);
    QStringList search(const QString &queryStr);
    bool hasIndex() const { return writer != nullptr; }

private:
    QString indexPath;
    Lucene::IndexWriterPtr writer;
    Lucene::SearcherPtr searcher;
    void initializeIndex();
};

#endif // SEARCHENGINE_H 