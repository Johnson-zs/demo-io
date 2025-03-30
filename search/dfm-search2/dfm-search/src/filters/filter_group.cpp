#include <dfm-search/search_filter.h>

#include <QDebug>

namespace DFM {
namespace Search {

FilterGroup::FilterGroup(Operation op)
    : operation_(op)
{
}

void FilterGroup::addFilter(std::shared_ptr<Filter> filter)
{
    if (filter) {
        filters_.push_back(std::move(filter));
    }
}

bool FilterGroup::matches(const QFileInfo& fileInfo) const
{
    // 如果没有过滤器，默认匹配
    if (filters_.empty()) {
        return true;
    }
    
    if (operation_ == Operation::AND) {
        // 所有过滤器都必须匹配
        for (const auto& filter : filters_) {
            if (!filter->matches(fileInfo)) {
                return false;
            }
        }
        return true;
    } else { // Operation::OR
        // 只要有一个过滤器匹配即可
        for (const auto& filter : filters_) {
            if (filter->matches(fileInfo)) {
                return true;
            }
        }
        return false;
    }
}

QString FilterGroup::description() const
{
    QStringList descriptions;
    for (const auto& filter : filters_) {
        descriptions.append(filter->description());
    }
    
    QString opStr = (operation_ == Operation::AND) ? "AND" : "OR";
    return QString("过滤器组(%1): [%2]").arg(opStr, descriptions.join(", "));
}

// FilterFactory的过滤器组创建方法
std::shared_ptr<FilterGroup> FilterFactory::createFilterGroup(FilterGroup::Operation op)
{
    return std::make_shared<FilterGroup>(op);
}

// 从SearchOptions创建过滤器
std::shared_ptr<Filter> FilterFactory::createFromOptions(const SearchOptions& options)
{
    // 创建一个AND过滤器组
    auto group = createFilterGroup(FilterGroup::Operation::AND);
    
    // 添加路径过滤器
    if (!options.searchPaths.isEmpty() || !options.excludePaths.isEmpty()) {
        group->addFilter(createPathFilter(options.searchPaths, options.excludePaths));
    }
    
    // 添加文件类型过滤器
    if (!options.fileFilters.isEmpty()) {
        group->addFilter(createExtensionFilter(options.fileFilters));
    }
    
    // 返回创建的过滤器组
    return group;
}

} // namespace Search
} // namespace DFM 