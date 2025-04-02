#include "searchoptionsdata.h"
#include <QDir>

namespace DFM6 {
namespace Search {

SearchOptionsData::SearchOptionsData()
    : method(SearchMethod::Indexed),
      caseSensitive(false),
      searchPath(QDir::homePath()),
      includeHidden(false),
      maxResults(1000)
{
}

SearchOptionsData::SearchOptionsData(const SearchOptionsData &other)
    : method(other.method),
      caseSensitive(other.caseSensitive),
      searchPath(other.searchPath),
      excludePaths(other.excludePaths),
      includeHidden(other.includeHidden),
      maxResults(other.maxResults),
      customOptions(other.customOptions)
{
}

}  // namespace Search
}  // namespace DFM6 