#include "qsearch/search_result.h"
#include <QVector>
#include <algorithm>

namespace QSearch {

struct SearchResult::Impl {
    QVector<ResultItem> items;
    int currentPage = 0;
    int pageSize = 50;
};

SearchResult::SearchResult() : d(new Impl) {
}

void SearchResult::addItem(const ResultItem& item) {
    d->items.append(item);
}

QVector<ResultItem> SearchResult::items() const {
    return d->items;
}

int SearchResult::count() const {
    return d->items.count();
}

bool SearchResult::isEmpty() const {
    return d->items.isEmpty();
}

void SearchResult::setPage(int page, int pageSize) {
    d->currentPage = page;
    d->pageSize = pageSize;
}

QVector<ResultItem> SearchResult::currentPage() const {
    int startIndex = d->currentPage * d->pageSize;
    if (startIndex >= d->items.size()) {
        return {};
    }
    
    int endIndex = qMin(startIndex + d->pageSize, d->items.size());
    return QVector<ResultItem>(d->items.begin() + startIndex, d->items.begin() + endIndex);
}

bool SearchResult::hasNextPage() const {
    return (d->currentPage + 1) * d->pageSize < d->items.size();
}

bool SearchResult::hasPreviousPage() const {
    return d->currentPage > 0;
}

void SearchResult::sort(SortField field, SortOrder order) {
    auto comparator = [field, order](const ResultItem& a, const ResultItem& b) {
        bool ascending = (order == SortOrder::Ascending);
        
        switch (field) {
            case SortField::Name:
                return ascending ? (a.name < b.name) : (a.name > b.name);
            case SortField::Path:
                return ascending ? (a.path < b.path) : (a.path > b.path);
            case SortField::Size:
                return ascending ? (a.size < b.size) : (a.size > b.size);
            case SortField::ModifiedTime:
                return ascending ? (a.modifiedTime < b.modifiedTime) : (a.modifiedTime > b.modifiedTime);
            default:
                return false;
        }
    };
    
    std::sort(d->items.begin(), d->items.end(), comparator);
}

SearchResult SearchResult::filtered(const std::function<bool(const ResultItem&)>& predicate) const {
    SearchResult result;
    for (const auto& item : d->items) {
        if (predicate(item)) {
            result.addItem(item);
        }
    }
    return result;
}

} // namespace QSearch 