#ifndef DFM6_CONTENT_SEARCH_API_H
#define DFM6_CONTENT_SEARCH_API_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 内容搜索API
 * 
 * 提供内容搜索特有的选项设置
 */
class DFM6_SEARCH_EXPORT ContentSearchAPI
{
public:
    /**
     * @brief 构造函数
     * 
     * @param options 要操作的搜索选项对象
     */
    explicit ContentSearchAPI(SearchOptions& options);
    
    /**
     * @brief 设置文件类型过滤器
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
    
private:
    SearchOptions& m_options;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_CONTENT_SEARCH_API_H 