#include <dfm6-search/searchoptions.h>
#include "searchoptionsdata.h"

namespace DFM6 {
namespace Search {

SearchOptions::SearchOptions()
    : d(std::make_unique<SearchOptionsData>())
{
}

SearchOptions::SearchOptions(const SearchOptions &other)
    : d(std::make_unique<SearchOptionsData>(*other.d))
{
}

SearchOptions::SearchOptions(SearchOptions &&other) noexcept
    : d(std::move(other.d))
{
}

SearchOptions::~SearchOptions() = default;

SearchOptions& SearchOptions::operator=(const SearchOptions &other)
{
    if (this != &other) {
        d = std::make_unique<SearchOptionsData>(*other.d);
    }
    return *this;
}

SearchOptions& SearchOptions::operator=(SearchOptions &&other) noexcept
{
    if (this != &other) {
        d = std::move(other.d);
    }
    return *this;
}

SearchMethod SearchOptions::method() const
{
    return d->method;
}

void SearchOptions::setMethod(SearchMethod method)
{
    d->method = method;
}

bool SearchOptions::caseSensitive() const
{
    return d->caseSensitive;
}

void SearchOptions::setCaseSensitive(bool sensitive)
{
    d->caseSensitive = sensitive;
}

QString SearchOptions::searchPath() const
{
    return d->searchPath;
}

void SearchOptions::setSearchPath(const QString &path)
{
    d->searchPath = path;
}

QStringList SearchOptions::excludePaths() const
{
    return d->excludePaths;
}

void SearchOptions::setExcludePaths(const QStringList &paths)
{
    d->excludePaths = paths;
}

void SearchOptions::addExcludePath(const QString &path)
{
    if (!d->excludePaths.contains(path)) {
        d->excludePaths.append(path);
    }
}

void SearchOptions::setIncludeHidden(bool include)
{
    d->includeHidden = include;
}

bool SearchOptions::includeHidden() const
{
    return d->includeHidden;
}

int SearchOptions::maxResults() const
{
    return d->maxResults;
}

void SearchOptions::setMaxResults(int count)
{
    d->maxResults = count;
}

void SearchOptions::setCustomOption(const QString &key, const QVariant &value)
{
    d->customOptions[key] = value;
}

QVariant SearchOptions::customOption(const QString &key) const
{
    return d->customOptions.value(key);
}

bool SearchOptions::hasCustomOption(const QString &key) const
{
    return d->customOptions.contains(key);
}

QVariantMap SearchOptions::customOptions() const
{
    return d->customOptions;
}

std::unique_ptr<SearchOptions> SearchOptions::clone() const
{
    return std::make_unique<SearchOptions>(*this);
}

SearchOptionsData* SearchOptions::data()
{
    return d.get();
}

const SearchOptionsData* SearchOptions::data() const
{
    return d.get();
}

}  // namespace Search
}  // namespace DFM6 