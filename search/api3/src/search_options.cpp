#include "qsearch/search_options.h"

namespace QSearch {

struct SearchOptions::Impl {
    int maxResults = 1000;
    SortOrder sortOrder = SortOrder::NoSort;
    SearchMode searchMode = SearchMode::Normal;
    bool caseSensitive = false;
    int threadCount = 2;
    int timeoutMs = 30000;  // 默认30秒超时
    bool lowPriority = false;
    QMap<QString, QVariant> customOptions;
};

SearchOptions::SearchOptions() : d(new Impl) {
}

SearchOptions& SearchOptions::setMaxResults(int maxResults) {
    d->maxResults = maxResults;
    return *this;
}

SearchOptions& SearchOptions::setSortOrder(SortOrder order) {
    d->sortOrder = order;
    return *this;
}

SearchOptions& SearchOptions::setSearchMode(SearchMode mode) {
    d->searchMode = mode;
    return *this;
}

SearchOptions& SearchOptions::setCaseSensitive(bool sensitive) {
    d->caseSensitive = sensitive;
    return *this;
}

SearchOptions& SearchOptions::setThreadCount(int count) {
    d->threadCount = count;
    return *this;
}

SearchOptions& SearchOptions::setTimeoutMs(int timeout) {
    d->timeoutMs = timeout;
    return *this;
}

SearchOptions& SearchOptions::setLowPriority(bool lowPriority) {
    d->lowPriority = lowPriority;
    return *this;
}

SearchOptions& SearchOptions::setOption(const QString& key, const QVariant& value) {
    d->customOptions[key] = value;
    return *this;
}

QVariant SearchOptions::option(const QString& key) const {
    return d->customOptions.value(key);
}

bool SearchOptions::hasOption(const QString& key) const {
    return d->customOptions.contains(key);
}

int SearchOptions::maxResults() const {
    return d->maxResults;
}

SortOrder SearchOptions::sortOrder() const {
    return d->sortOrder;
}

SearchMode SearchOptions::searchMode() const {
    return d->searchMode;
}

bool SearchOptions::isCaseSensitive() const {
    return d->caseSensitive;
}

int SearchOptions::threadCount() const {
    return d->threadCount;
}

int SearchOptions::timeoutMs() const {
    return d->timeoutMs;
}

bool SearchOptions::isLowPriority() const {
    return d->lowPriority;
}

} // namespace QSearch 