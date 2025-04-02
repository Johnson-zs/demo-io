#ifndef DFM6_SEARCH_RESULT_H
#define DFM6_SEARCH_RESULT_H

#include <dfm6-search/global.h>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <memory>

namespace DFM6 {
namespace Search {

class SearchResultData;

/**
 * @brief 搜索结果类
 * 
 * 封装单个搜索结果项的数据
 */
class DFM6_SEARCH_EXPORT SearchResult
{
public:
    /**
     * @brief 构造函数
     */
    SearchResult();
    
    /**
     * @brief 构造函数
     * 
     * @param path 文件路径
     */
    explicit SearchResult(const QString &path);
    
    /**
     * @brief 复制构造函数
     */
    SearchResult(const SearchResult &other);
    
    /**
     * @brief 移动构造函数
     */
    SearchResult(SearchResult &&other) noexcept;
    
    /**
     * @brief 析构函数
     */
    ~SearchResult();
    
    /**
     * @brief 赋值运算符
     */
    SearchResult& operator=(const SearchResult &other);
    
    /**
     * @brief 移动赋值运算符
     */
    SearchResult& operator=(SearchResult &&other) noexcept;
    
    /**
     * @brief 获取文件路径
     */
    QString path() const;
    
    /**
     * @brief 设置文件路径
     */
    void setPath(const QString &path);
    
    /**
     * @brief 获取文件名
     */
    QString fileName() const;
    
    /**
     * @brief 获取文件修改时间
     */
    QDateTime modifiedTime() const;
    
    /**
     * @brief 设置文件修改时间
     */
    void setModifiedTime(const QDateTime &time);
    
    /**
     * @brief 获取文件大小
     */
    qint64 size() const;
    
    /**
     * @brief 设置文件大小
     */
    void setSize(qint64 size);
    
    /**
     * @brief 获取高亮内容（用于全文搜索）
     */
    QString highlightedContent() const;
    
    /**
     * @brief 设置高亮内容
     */
    void setHighlightedContent(const QString &content);
    
    /**
     * @brief 获取匹配分数
     */
    float score() const;
    
    /**
     * @brief 设置匹配分数
     */
    void setScore(float score);
    
    /**
     * @brief 判断是否为目录
     */
    bool isDirectory() const;
    
    /**
     * @brief 设置是否为目录
     */
    void setIsDirectory(bool isDir);
    
    /**
     * @brief 设置自定义属性
     */
    void setCustomAttribute(const QString &key, const QVariant &value);
    
    /**
     * @brief 获取自定义属性
     */
    QVariant customAttribute(const QString &key) const;
    
    /**
     * @brief 获取所有自定义属性
     */
    QVariantMap customAttributes() const;

private:
    std::unique_ptr<SearchResultData> d;  // PIMPL
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_RESULT_H 