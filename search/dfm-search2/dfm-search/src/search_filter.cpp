#include <dfm-search/search_filter.h>

#include <QDebug>

namespace DFM {
namespace Search {

// FilterFactory实现
std::shared_ptr<Filter> FilterFactory::createFromOptions(const SearchOptions& options)
{
    // 创建一个AND过滤器组
    auto group = createFilterGroup(FilterGroup::Operation::AND);
    
    // 添加路径过滤器
    if (!options.searchPaths().isEmpty() || !options.excludePaths().isEmpty()) {
        group->addFilter(createPathFilter(options.searchPaths(), options.excludePaths()));
    }
    
    // 添加文件类型过滤器
    if (!options.fileFilters().isEmpty()) {
        group->addFilter(createExtensionFilter(options.fileFilters()));
    }
    
    // 添加其他过滤器逻辑...
    
    return group;
}

} // namespace Search
} // namespace DFM 