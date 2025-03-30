#pragma once

#include "global.h"
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <memory>

namespace DFMSearch {

class SearchResultPrivate;

/**
 * @brief 搜索结果基类
 * 
 * 提供通用的搜索结果接口，并可通过继承扩展特定类型结果
 */
class DFMSEARCH_EXPORT SearchResult
{
public:
    /**
     * @brief 构造函数
     * @param type 结果类型
     */
    explicit SearchResult(ResultType type = ResultType::File);
    
    /**
     * @brief 拷贝构造函数
     */
    SearchResult(const SearchResult& other);
    
    /**
     * @brief 移动构造函数
     */
    SearchResult(SearchResult&& other) noexcept;
    
    /**
     * @brief 拷贝赋值运算符
     */
    SearchResult& operator=(const SearchResult& other);
    
    /**
     * @brief 移动赋值运算符
     */
    SearchResult& operator=(SearchResult&& other) noexcept;
    
    /**
     * @brief 析构函数
     */
    virtual ~SearchResult();

    /**
     * @brief 获取结果类型
     * @return 结果类型
     */
    ResultType type() const;
    
    /**
     * @brief 设置结果标题
     * @param title 标题
     */
    void setTitle(const QString& title);
    
    /**
     * @brief 获取结果标题
     * @return 标题
     */
    QString title() const;
    
    /**
     * @brief 设置结果路径
     * @param path 路径
     */
    void setPath(const QString& path);
    
    /**
     * @brief 获取结果路径
     * @return 路径
     */
    QString path() const;
    
    /**
     * @brief 设置结果描述
     * @param description 描述
     */
    void setDescription(const QString& description);
    
    /**
     * @brief 获取结果描述
     * @return 描述
     */
    QString description() const;
    
    /**
     * @brief 设置结果匹配度/权重
     * @param score 匹配度得分（0.0-1.0）
     */
    void setScore(double score);
    
    /**
     * @brief 获取结果匹配度/权重
     * @return 匹配度得分
     */
    double score() const;
    
    /**
     * @brief 设置额外数据
     * @param key 数据键名
     * @param value 数据值
     */
    void setData(const QString& key, const QVariant& value);
    
    /**
     * @brief 获取额外数据
     * @param key 数据键名
     * @return 数据值
     */
    QVariant data(const QString& key) const;
    
    /**
     * @brief 检查是否包含指定额外数据
     * @param key 数据键名
     * @return 是否包含
     */
    bool hasData(const QString& key) const;

    /**
     * @brief 转换为可打印字符串
     * @return 字符串形式
     */
    virtual QString toString() const;

private:
    std::unique_ptr<SearchResultPrivate> d;
};

/**
 * @brief 文件搜索结果类
 */
class DFMSEARCH_EXPORT FileSearchResult : public SearchResult
{
public:
    /**
     * @brief 构造函数
     */
    FileSearchResult();
    
    /**
     * @brief 设置文件大小
     * @param size 文件大小（字节）
     */
    void setFileSize(qint64 size);
    
    /**
     * @brief 获取文件大小
     * @return 文件大小
     */
    qint64 fileSize() const;
    
    /**
     * @brief 设置文件修改时间
     * @param time 修改时间
     */
    void setModifiedTime(const QDateTime& time);
    
    /**
     * @brief 获取文件修改时间
     * @return 修改时间
     */
    QDateTime modifiedTime() const;
    
    /**
     * @brief 转换为可打印字符串
     * @return 字符串形式
     */
    QString toString() const override;
};

/**
 * @brief 文件内容搜索结果类
 */
class DFMSEARCH_EXPORT ContentSearchResult : public SearchResult
{
public:
    /**
     * @brief 构造函数
     */
    ContentSearchResult();
    
    /**
     * @brief 设置匹配内容
     * @param content 匹配的内容
     */
    void setMatchedContent(const QString& content);
    
    /**
     * @brief 获取匹配内容
     * @return 匹配的内容
     */
    QString matchedContent() const;
    
    /**
     * @brief 设置匹配行号
     * @param lineNumber 行号
     */
    void setLineNumber(int lineNumber);
    
    /**
     * @brief 获取匹配行号
     * @return 行号
     */
    int lineNumber() const;
    
    /**
     * @brief 转换为可打印字符串
     * @return 字符串形式
     */
    QString toString() const override;
};

/**
 * @brief 应用程序搜索结果类
 */
class DFMSEARCH_EXPORT AppSearchResult : public SearchResult
{
public:
    /**
     * @brief 构造函数
     */
    AppSearchResult();
    
    /**
     * @brief 设置应用图标路径
     * @param iconPath 图标路径
     */
    void setIconPath(const QString& iconPath);
    
    /**
     * @brief 获取应用图标路径
     * @return 图标路径
     */
    QString iconPath() const;
    
    /**
     * @brief 设置应用执行命令
     * @param command 执行命令
     */
    void setExecuteCommand(const QString& command);
    
    /**
     * @brief 获取应用执行命令
     * @return 执行命令
     */
    QString executeCommand() const;
    
    /**
     * @brief 转换为可打印字符串
     * @return 字符串形式
     */
    QString toString() const override;
};

} // namespace DFMSearch 