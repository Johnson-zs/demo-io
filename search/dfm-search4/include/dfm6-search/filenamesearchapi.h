#ifndef DFM6_FILENAME_SEARCH_API_H
#define DFM6_FILENAME_SEARCH_API_H

#include <dfm6-search/global.h>
#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 文件名搜索API
 * 
 * 提供文件名搜索特有的选项设置
 */
class DFM6_SEARCH_EXPORT FileNameSearchAPI
{
public:
    /**
     * @brief 构造函数
     * 
     * @param options 要操作的搜索选项对象
     */
    explicit FileNameSearchAPI(SearchOptions& options);
    
    /**
     * @brief 设置是否启用拼音搜索
     */
    void setPinyinEnabled(bool enabled);
    
    /**
     * @brief 获取是否启用拼音搜索
     */
    bool pinyinEnabled() const;
    
    /**
     * @brief 设置模糊搜索
     */
    void setFuzzySearch(bool enabled);
    
    /**
     * @brief 获取是否启用模糊搜索
     */
    bool fuzzySearch() const;
    
    /**
     * @brief 设置文件类型过滤
     */
    void setFileTypes(const QStringList &types);
    
    /**
     * @brief 获取文件类型过滤
     */
    QStringList fileTypes() const;
    
private:
    SearchOptions& m_options;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_FILENAME_SEARCH_API_H 