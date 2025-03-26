#include "qsearch/search_query.h"
#include <QDateTime>

namespace QSearch {

struct SearchQuery::Impl {
    QString queryText;
    QueryType queryType = QueryType::Filename;
    MatchType matchType = MatchType::Contains;
    QStringList searchPaths;
    QStringList fileFilters;
    qint64 minSize = -1;
    qint64 maxSize = -1;
    QDateTime startTime;
    QDateTime endTime;
    QMap<QString, QVariant> options;
    QList<SearchQuery> andQueries;
    QList<SearchQuery> orQueries;
    QList<SearchQuery> notQueries;
};

SearchQuery::SearchQuery() : d(new Impl) {
}

SearchQuery::SearchQuery(const QString& queryText) : d(new Impl) {
    d->queryText = queryText;
}

SearchQuery& SearchQuery::setType(QueryType type) {
    d->queryType = type;
    return *this;
}

SearchQuery& SearchQuery::setMatchType(MatchType matchType) {
    d->matchType = matchType;
    return *this;
}

SearchQuery& SearchQuery::setText(const QString& text) {
    d->queryText = text;
    return *this;
}

SearchQuery& SearchQuery::setPaths(const QStringList& paths) {
    d->searchPaths = paths;
    return *this;
}

SearchQuery& SearchQuery::addPath(const QString& path) {
    d->searchPaths.append(path);
    return *this;
}

SearchQuery& SearchQuery::setFileFilters(const QStringList& filters) {
    d->fileFilters = filters;
    return *this;
}

SearchQuery& SearchQuery::addFileFilter(const QString& filter) {
    d->fileFilters.append(filter);
    return *this;
}

SearchQuery& SearchQuery::setSizeLimit(qint64 minSize, qint64 maxSize) {
    d->minSize = minSize;
    d->maxSize = maxSize;
    return *this;
}

SearchQuery& SearchQuery::setTimeLimit(const QDateTime& startTime, const QDateTime& endTime) {
    d->startTime = startTime;
    d->endTime = endTime;
    return *this;
}

SearchQuery& SearchQuery::setOption(const QString& key, const QVariant& value) {
    d->options[key] = value;
    return *this;
}

QVariant SearchQuery::option(const QString& key) const {
    return d->options.value(key);
}

SearchQuery& SearchQuery::and_(const SearchQuery& query) {
    d->andQueries.append(query);
    return *this;
}

SearchQuery& SearchQuery::or_(const SearchQuery& query) {
    d->orQueries.append(query);
    return *this;
}

SearchQuery& SearchQuery::not_(const SearchQuery& query) {
    d->notQueries.append(query);
    return *this;
}

QString SearchQuery::text() const {
    return d->queryText;
}

QueryType SearchQuery::type() const {
    return d->queryType;
}

MatchType SearchQuery::matchType() const {
    return d->matchType;
}

QStringList SearchQuery::paths() const {
    return d->searchPaths;
}

QStringList SearchQuery::fileFilters() const {
    return d->fileFilters;
}

} // namespace QSearch 