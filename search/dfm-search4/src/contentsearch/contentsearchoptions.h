#ifndef CONTENT_SEARCH_OPTIONS_H
#define CONTENT_SEARCH_OPTIONS_H

#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 内容搜索专用选项
 * 
 * 提供内容搜索特有的选项设置
 */
class ContentSearchOptions : public SearchOptions
{
public:
    /**
     * @brief 构造函数
     */
    ContentSearchOptions();
    
    /**
     * @brief 拷贝构造函数
     */
    ContentSearchOptions(const ContentSearchOptions &other);
    
    /**
     * @brief 移动构造函数
     */
    ContentSearchOptions(ContentSearchOptions &&other) noexcept;
    
    /**
     * @brief 从基类构造
     */
    explicit ContentSearchOptions(const SearchOptions &options);
    
    /**
     * @brief 析构函数
     */
    ~ContentSearchOptions() override;
    
    /**
     * @brief 赋值操作符
     */
    ContentSearchOptions& operator=(const ContentSearchOptions &other);
    
    /**
     * @brief 移动赋值操作符
     */
    ContentSearchOptions& operator=(ContentSearchOptions &&other) noexcept;
    
    /**
     * @brief 设置是否只搜索特定文件类型
     */
    void setFileTypeFilters(const QStringList &extensions);
    
    /**
     * @brief 获取文件类型过滤器
     */
    QStringList fileTypeFilters() const;
    
    /**
     * @brief 设置最大内容预览长度
     */
    void setMaxPreviewLength(int length);
    
    /**
     * @brief 获取最大内容预览长度
     */
    int maxPreviewLength() const;
    
    /**
     * @brief 设置是否搜索二进制文件
     */
    void setSearchBinaryFiles(bool enabled);
    
    /**
     * @brief 是否搜索二进制文件
     */
    bool searchBinaryFiles() const;
    
    /**
     * @brief 设置索引路径
     */
    void setIndexPath(const QString &path);
    
    /**
     * @brief 获取索引路径
     */
    QString indexPath() const;
    
    /**
     * @brief 克隆当前选项
     */
    std::unique_ptr<SearchOptions> clone() const override;
};

}  // namespace Search
}  // namespace DFM6

#endif // CONTENT_SEARCH_OPTIONS_H 