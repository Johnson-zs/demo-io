#ifndef SEARCH_OPTIONS_DATA_H
#define SEARCH_OPTIONS_DATA_H

#include <dfm6-search/searchoptions.h>
#include <QVariantMap>
#include <QStringList>

namespace DFM6 {
namespace Search {

/**
 * @brief SearchOptions的私有实现类
 */
class SearchOptionsData
{
public:
    SearchOptionsData();
    SearchOptionsData(const SearchOptionsData &other);
    
    // 公共数据字段
    SearchMethod method;
    bool caseSensitive;
    QString searchPath;
    QStringList excludePaths;
    bool includeHidden;
    int maxResults;
    QVariantMap customOptions;
    bool recursive = true; // 默认递归搜索
};

}  // namespace Search
}  // namespace DFM6

#endif // SEARCH_OPTIONS_DATA_H 