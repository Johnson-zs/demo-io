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
class FileNameSearchResultData;
class ContentSearchResultData;

/**
 * @brief 搜索结果基类
 * 
 * 定义所有搜索结果共有的基本属性和方法
 */
class DFM6_SEARCH_EXPORT SearchResult
{
public:
    /**
     * @brief 构造函数
     */
    SearchResult();
    
    /**
     * @brief 使用路径构造
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
     * @brief 虚析构函数
     */
    virtual ~SearchResult();
    
    /**
     * @brief 赋值操作符
     */
    SearchResult& operator=(const SearchResult &other);
    
    /**
     * @brief 移动赋值操作符
     */
    SearchResult& operator=(SearchResult &&other) noexcept;
    
    /**
     * @brief 获取搜索结果类型
     */
    virtual SearchType resultType() const = 0;
    
    // 基本文件属性
    QString path() const;
    void setPath(const QString &path);
    
    QString fileName() const;
    
    qint64 size() const;
    void setSize(qint64 size);
    
    QDateTime modifiedTime() const;
    void setModifiedTime(const QDateTime &time);
    
    bool isDirectory() const;
    void setIsDirectory(bool isDir);
    
    // 通用评分
    float score() const;
    void setScore(float score);
    
    // 自定义属性
    void setCustomAttribute(const QString &key, const QVariant &value);
    QVariant customAttribute(const QString &key) const;
    bool hasCustomAttribute(const QString &key) const;
    QVariantMap customAttributes() const;
    
    /**
     * @brief 创建特定类型的搜索结果
     * 
     * 工厂方法，根据类型创建对应的搜索结果对象
     * 
     * @param type 搜索结果类型
     * @param path 文件路径
     * @return 对应类型的搜索结果对象
     */
    static std::unique_ptr<SearchResult> create(SearchType type, const QString &path);
    
    /**
     * @brief 克隆当前搜索结果
     */
    virtual std::unique_ptr<SearchResult> clone() const = 0;

protected:
    std::unique_ptr<SearchResultData> d;
};

/**
 * @brief 文件名搜索结果
 */
class DFM6_SEARCH_EXPORT FileNameSearchResult : public SearchResult
{
public:
    /**
     * @brief 默认构造函数
     */
    FileNameSearchResult();
    
    /**
     * @brief 使用路径构造
     */
    explicit FileNameSearchResult(const QString &path);
    
    /**
     * @brief 复制构造函数
     */
    FileNameSearchResult(const FileNameSearchResult &other);
    
    /**
     * @brief 基类复制构造函数
     */
    explicit FileNameSearchResult(const SearchResult &other);
    
    /**
     * @brief 获取搜索结果类型
     */
    SearchType resultType() const override { return SearchType::FileName; }
    
    /**
     * @brief 克隆当前搜索结果
     */
    std::unique_ptr<SearchResult> clone() const override;
    
    // 文件名搜索特有属性
    QString matchType() const;
    void setMatchType(const QString &type);

private:
    // 辅助函数，获取类型化的数据指针
    FileNameSearchResultData* data();
    const FileNameSearchResultData* data() const;
};

/**
 * @brief 内容搜索结果
 */
class DFM6_SEARCH_EXPORT ContentSearchResult : public SearchResult
{
public:
    /**
     * @brief 默认构造函数
     */
    ContentSearchResult();
    
    /**
     * @brief 使用路径构造
     */
    explicit ContentSearchResult(const QString &path);
    
    /**
     * @brief 复制构造函数
     */
    ContentSearchResult(const ContentSearchResult &other);
    
    /**
     * @brief 基类复制构造函数
     */
    explicit ContentSearchResult(const SearchResult &other);
    
    /**
     * @brief 获取搜索结果类型
     */
    SearchType resultType() const override { return SearchType::Content; }
    
    /**
     * @brief 克隆当前搜索结果
     */
    std::unique_ptr<SearchResult> clone() const override;
    
    // 内容搜索特有属性
    QString highlightedContent() const;
    void setHighlightedContent(const QString &content);
    
    int lineNumber() const;
    void setLineNumber(int line);
    
    int matchStart() const;
    void setMatchStart(int pos);
    
    int matchLength() const;
    void setMatchLength(int len);

private:
    // 辅助函数，获取类型化的数据指针
    ContentSearchResultData* data();
    const ContentSearchResultData* data() const;
};

}  // namespace Search
}  // namespace DFM6

#endif // DFM6_SEARCH_RESULT_H 