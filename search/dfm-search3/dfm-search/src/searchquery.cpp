#include "dfm-search/searchquery.h"
#include <type_traits> // 添加此头文件以支持 std::underlying_type_t

namespace DFMSearch {

// 私有实现类
class SearchQueryPrivate {
public:
    QString keyword;
    QStringList searchPaths;
    SearchMechanism mechanism = SearchMechanism::RealTime;
    SearchFlags flags = SearchFlag::None;
    int limit = 0; // 0表示无限制
};

SearchQuery::SearchQuery(const QString& keyword)
    : d(std::make_unique<SearchQueryPrivate>())
{
    d->keyword = keyword;
}

SearchQuery::SearchQuery(const SearchQuery& other)
    : d(std::make_unique<SearchQueryPrivate>(*other.d))
{
}

SearchQuery::SearchQuery(SearchQuery&& other) noexcept
    : d(std::move(other.d))
{
}

SearchQuery& SearchQuery::operator=(const SearchQuery& other)
{
    if (this != &other) {
        d = std::make_unique<SearchQueryPrivate>(*other.d);
    }
    return *this;
}

SearchQuery& SearchQuery::operator=(SearchQuery&& other) noexcept
{
    if (this != &other) {
        d = std::move(other.d);
    }
    return *this;
}

SearchQuery::~SearchQuery() = default;

void SearchQuery::setKeyword(const QString& keyword)
{
    d->keyword = keyword;
}

QString SearchQuery::keyword() const
{
    return d->keyword;
}

void SearchQuery::setSearchPaths(const QStringList& paths)
{
    d->searchPaths = paths;
}

QStringList SearchQuery::searchPaths() const
{
    return d->searchPaths;
}

void SearchQuery::addSearchPath(const QString& path)
{
    if (!d->searchPaths.contains(path)) {
        d->searchPaths.append(path);
    }
}

void SearchQuery::setMechanism(SearchMechanism mechanism)
{
    d->mechanism = mechanism;
}

SearchMechanism SearchQuery::mechanism() const
{
    return d->mechanism;
}

void SearchQuery::setFlags(SearchFlags flags)
{
    d->flags = flags;
}

SearchFlags SearchQuery::flags() const
{
    return d->flags;
}

void SearchQuery::addFlag(SearchFlag flag)
{
    d->flags |= flag;
}

void SearchQuery::removeFlag(SearchFlag flag)
{
    d->flags &= ~static_cast<std::underlying_type_t<SearchFlag>>(flag);
}

bool SearchQuery::hasFlag(SearchFlag flag) const
{
    return d->flags.testFlag(flag);
}

void SearchQuery::setLimit(int limit)
{
    d->limit = limit;
}

int SearchQuery::limit() const
{
    return d->limit;
}

} // namespace DFMSearch 