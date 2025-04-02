#ifndef FILENAME_SEARCH_OPTIONS_H
#define FILENAME_SEARCH_OPTIONS_H

#include <dfm6-search/searchoptions.h>

namespace DFM6 {
namespace Search {

/**
 * @brief 文件名搜索专用选项
 * 
 * 提供文件名搜索特有的选项设置
 */
class FileNameSearchOptions : public SearchOptions
{
public:
    /**
     * @brief 构造函数
     */
    FileNameSearchOptions();
    
    /**
     * @brief 拷贝构造函数
     */
    FileNameSearchOptions(const FileNameSearchOptions &other);
    
    /**
     * @brief 移动构造函数
     */
    FileNameSearchOptions(FileNameSearchOptions &&other) noexcept;
    
    /**
     * @brief 从基类构造
     */
    explicit FileNameSearchOptions(const SearchOptions &options);
    
    /**
     * @brief 析构函数
     */
    ~FileNameSearchOptions() override;
    
    /**
     * @brief 赋值操作符
     */
    FileNameSearchOptions& operator=(const FileNameSearchOptions &other);
    
    /**
     * @brief 移动赋值操作符
     */
    FileNameSearchOptions& operator=(FileNameSearchOptions &&other) noexcept;
    
    /**
     * @brief 启用拼音搜索
     */
    void setPinyinEnabled(bool enabled);
    
    /**
     * @brief 是否启用拼音搜索
     */
    bool pinyinEnabled() const;
    
    /**
     * @brief 设置模糊搜索
     */
    void setFuzzySearch(bool enabled);
    
    /**
     * @brief 是否启用模糊搜索
     */
    bool fuzzySearch() const;
    
    /**
     * @brief 筛选文件类型
     */
    void setFileTypes(const QStringList &types);
    
    /**
     * @brief 获取筛选的文件类型
     */
    QStringList fileTypes() const;
    
    /**
     * @brief 克隆当前选项
     */
    std::unique_ptr<SearchOptions> clone() const override;
};

}  // namespace Search
}  // namespace DFM6

#endif // FILENAME_SEARCH_OPTIONS_H 