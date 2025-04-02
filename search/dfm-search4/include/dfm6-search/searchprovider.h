#ifndef DFM6_SEARCH_PROVIDER_H
#define DFM6_SEARCH_PROVIDER_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchoptions.h>
#include <dfm6-search/searchresult.h>

#include <QObject>
#include <QString>
#include <memory>

namespace DFM6 {
namespace Search {

class SearchProviderData;

/**
 * @brief 搜索提供者接口
 * 
 * 用于插件系统，允许第三方扩展搜索功能
 */
class DFM6_SEARCH_EXPORT SearchProvider
{
public:
    virtual ~SearchProvider() = default;
    
    /**
     * @brief 获取提供者名称
     */
    virtual QString name() const = 0;
    
    /**
     * @brief 获取提供者支持的搜索类型
     */
    virtual SearchType supportedType() const = 0;
    
    /**
     * @brief 创建搜索引擎实例
     */
    virtual std::shared_ptr<AbstractSearchEngine> createEngine(QObject *parent = nullptr) = 0;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_PROVIDER_H 