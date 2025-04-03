#ifndef DESKTOP_SEARCH_OPTIONS_H
#define DESKTOP_SEARCH_OPTIONS_H

#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 桌面应用搜索专用选项
 *
 * 提供桌面应用搜索特有的选项设置
 */
class DesktopSearchOptions : public SearchOptions
{
public:
    /**
     * @brief 构造函数
     */
    DesktopSearchOptions();
    
    /**
     * @brief 拷贝构造函数
     */
    DesktopSearchOptions(const DesktopSearchOptions &other);
    
    /**
     * @brief 移动构造函数
     */
    DesktopSearchOptions(DesktopSearchOptions &&other) noexcept;
    
    /**
     * @brief 从基类构造
     */
    explicit DesktopSearchOptions(const SearchOptions &options);
    
    /**
     * @brief 析构函数
     */
    ~DesktopSearchOptions() override;
    
    /**
     * @brief 赋值操作符
     */
    DesktopSearchOptions& operator=(const DesktopSearchOptions &other);
    
    /**
     * @brief 移动赋值操作符
     */
    DesktopSearchOptions& operator=(DesktopSearchOptions &&other) noexcept;
    
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
    
    /**
     * @brief 克隆当前选项
     */
    std::unique_ptr<SearchOptions> clone() const override;
};

}  // namespace Search
}  // namespace DFM6

#endif // DESKTOP_SEARCH_OPTIONS_H 