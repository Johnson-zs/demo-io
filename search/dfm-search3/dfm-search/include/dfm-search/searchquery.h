#pragma once

#include "global.h"
#include <QString>
#include <QStringList>
#include <memory>

namespace DFMSearch {

class SearchQueryPrivate;

/**
 * @brief 搜索查询类，定义搜索的各种参数
 * 
 * 此类使用PIMPL模式以保证ABI兼容性
 */
class DFMSEARCH_EXPORT SearchQuery
{
public:
    /**
     * @brief 构造函数
     * @param keyword 搜索关键词
     */
    explicit SearchQuery(const QString& keyword = QString());
    
    /**
     * @brief 拷贝构造函数
     */
    SearchQuery(const SearchQuery& other);
    
    /**
     * @brief 移动构造函数
     */
    SearchQuery(SearchQuery&& other) noexcept;
    
    /**
     * @brief 拷贝赋值运算符
     */
    SearchQuery& operator=(const SearchQuery& other);
    
    /**
     * @brief 移动赋值运算符
     */
    SearchQuery& operator=(SearchQuery&& other) noexcept;
    
    /**
     * @brief 析构函数
     */
    ~SearchQuery();

    /**
     * @brief 设置搜索关键词
     * @param keyword 关键词
     */
    void setKeyword(const QString& keyword);
    
    /**
     * @brief 获取搜索关键词
     * @return 关键词
     */
    QString keyword() const;
    
    /**
     * @brief 设置搜索路径
     * @param paths 路径列表
     */
    void setSearchPaths(const QStringList& paths);
    
    /**
     * @brief 获取搜索路径
     * @return 路径列表
     */
    QStringList searchPaths() const;
    
    /**
     * @brief 添加搜索路径
     * @param path 路径
     */
    void addSearchPath(const QString& path);
    
    /**
     * @brief 设置搜索机制
     * @param mechanism 搜索机制
     */
    void setMechanism(SearchMechanism mechanism);
    
    /**
     * @brief 获取搜索机制
     * @return 搜索机制
     */
    SearchMechanism mechanism() const;
    
    /**
     * @brief 设置搜索选项
     * @param flags 搜索选项标志
     */
    void setFlags(SearchFlags flags);
    
    /**
     * @brief 获取搜索选项
     * @return 搜索选项标志
     */
    SearchFlags flags() const;
    
    /**
     * @brief 添加搜索选项
     * @param flag 搜索选项标志
     */
    void addFlag(SearchFlag flag);
    
    /**
     * @brief 移除搜索选项
     * @param flag 搜索选项标志
     */
    void removeFlag(SearchFlag flag);
    
    /**
     * @brief 检查是否设置了指定搜索选项
     * @param flag 搜索选项标志
     * @return 是否设置
     */
    bool hasFlag(SearchFlag flag) const;
    
    /**
     * @brief 设置结果数量限制
     * @param limit 限制数量，0表示无限制
     */
    void setLimit(int limit);
    
    /**
     * @brief 获取结果数量限制
     * @return 限制数量
     */
    int limit() const;

private:
    std::unique_ptr<SearchQueryPrivate> d;
};

} // namespace DFMSearch 