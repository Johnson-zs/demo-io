#ifndef DFM6_DESKTOP_SEARCH_API_H
#define DFM6_DESKTOP_SEARCH_API_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 桌面应用搜索API
 * 
 * 提供桌面应用搜索特有的选项设置
 */
class DFM6_SEARCH_EXPORT DesktopSearchAPI
{
public:
    /**
     * @brief 构造函数
     * 
     * @param options 要操作的搜索选项对象
     */
    explicit DesktopSearchAPI(SearchOptions& options);
    
    /**
     * @brief 设置是否搜索名称
     */
    void setSearchName(bool enabled);
    
    /**
     * @brief 是否搜索名称
     */
    bool searchName() const;
    
    /**
     * @brief 设置是否搜索描述
     */
    void setSearchDescription(bool enabled);
    
    /**
     * @brief 是否搜索描述
     */
    bool searchDescription() const;
    
    /**
     * @brief 设置是否搜索关键词
     */
    void setSearchKeywords(bool enabled);
    
    /**
     * @brief 是否搜索关键词
     */
    bool searchKeywords() const;
    
    /**
     * @brief 设置是否仅搜索显示的应用
     */
    void setOnlyShowVisible(bool enabled);
    
    /**
     * @brief 是否仅搜索显示的应用
     */
    bool onlyShowVisible() const;
    
    /**
     * @brief 设置应用类别过滤
     */
    void setCategories(const QStringList &categories);
    
    /**
     * @brief 获取应用类别过滤
     */
    QStringList categories() const;
    
    /**
     * @brief 设置是否排序结果
     */
    void setSortResults(bool enabled);
    
    /**
     * @brief 是否排序结果
     */
    bool sortResults() const;
    
private:
    SearchOptions& m_options;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_DESKTOP_SEARCH_API_H 