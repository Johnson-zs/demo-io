#include <dfm6-search/searchquery.h>
#include <QStringList>

namespace DFM6 {
namespace Search {

// 前向声明私有实现类
class SearchQueryData {
public:
    SearchQueryData()
        : type(SearchQuery::Type::Simple),
          booleanOp(SearchQuery::BooleanOperator::AND),
          pinyinEnabled(false)
    {
    }
    
    SearchQueryData(const QString &keyword, SearchQuery::Type type = SearchQuery::Type::Simple)
        : keyword(keyword),
          type(type),
          booleanOp(SearchQuery::BooleanOperator::AND),
          pinyinEnabled(false)
    {
    }
    
    SearchQueryData(const SearchQueryData &other)
        : keyword(other.keyword),
          type(other.type),
          booleanOp(other.booleanOp),
          subQueries(other.subQueries),
          pinyinEnabled(other.pinyinEnabled)
    {
    }
    
    QString keyword;
    SearchQuery::Type type;
    SearchQuery::BooleanOperator booleanOp;
    QList<SearchQuery> subQueries;
    bool pinyinEnabled;
};

SearchQuery::SearchQuery()
    : d(std::make_unique<SearchQueryData>())
{
}

SearchQuery::SearchQuery(const QString &keyword)
    : d(std::make_unique<SearchQueryData>(keyword))
{
}

SearchQuery::SearchQuery(const QString &keyword, Type type)
    : d(std::make_unique<SearchQueryData>(keyword, type))
{
}

SearchQuery::SearchQuery(const SearchQuery &other)
    : d(std::make_unique<SearchQueryData>(*other.d))
{
}

SearchQuery::SearchQuery(SearchQuery &&other) noexcept
    : d(std::move(other.d))
{
}

SearchQuery::~SearchQuery() = default;

SearchQuery& SearchQuery::operator=(const SearchQuery &other)
{
    if (this != &other) {
        d = std::make_unique<SearchQueryData>(*other.d);
    }
    return *this;
}

SearchQuery& SearchQuery::operator=(SearchQuery &&other) noexcept
{
    if (this != &other) {
        d = std::move(other.d);
    }
    return *this;
}

QString SearchQuery::keyword() const
{
    return d->keyword;
}

void SearchQuery::setKeyword(const QString &keyword)
{
    d->keyword = keyword;
}

SearchQuery::Type SearchQuery::type() const
{
    return d->type;
}

void SearchQuery::setType(Type type)
{
    d->type = type;
}

SearchQuery::BooleanOperator SearchQuery::booleanOperator() const
{
    return d->booleanOp;
}

void SearchQuery::setBooleanOperator(BooleanOperator op)
{
    d->booleanOp = op;
}

void SearchQuery::addSubQuery(const SearchQuery &query)
{
    d->subQueries.append(query);
}

QList<SearchQuery> SearchQuery::subQueries() const
{
    return d->subQueries;
}

void SearchQuery::clearSubQueries()
{
    d->subQueries.clear();
}

bool SearchQuery::pinyinEnabled() const
{
    return d->pinyinEnabled;
}

void SearchQuery::setPinyinEnabled(bool enabled)
{
    d->pinyinEnabled = enabled;
}

SearchQuery SearchQuery::createSimpleQuery(const QString &keyword)
{
    return SearchQuery(keyword, Type::Simple);
}

SearchQuery SearchQuery::createBooleanQuery(const QStringList &keywords, BooleanOperator op)
{
    SearchQuery query;
    query.setType(Type::Boolean);
    query.setBooleanOperator(op);
    
    for (const QString &keyword : keywords) {
        query.addSubQuery(SearchQuery(keyword, Type::Simple));
    }
    
    return query;
}

SearchQuery SearchQuery::createWildcardQuery(const QString &pattern)
{
    return SearchQuery(pattern, Type::Wildcard);
}

SearchQuery SearchQuery::createFuzzyQuery(const QString &keyword)
{
    return SearchQuery(keyword, Type::Fuzzy);
}

SearchQuery SearchQuery::createRegexQuery(const QString &pattern)
{
    return SearchQuery(pattern, Type::Regex);
}

}  // namespace Search
}  // namespace DFM6 