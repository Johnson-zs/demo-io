#ifndef SEARCH_RESULT_DATA_H
#define SEARCH_RESULT_DATA_H

#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>

namespace DFM6 {
namespace Search {

/**
 * @brief SearchResult的基本私有实现类
 * 
 * 只包含所有搜索结果类型共有的数据
 */
class SearchResultData
{
public:
    SearchResultData();
    SearchResultData(const QString &path);
    SearchResultData(const SearchResultData &other);
    virtual ~SearchResultData() = default;
    
    // 公共数据字段
    QString path;
    QDateTime modifiedTime;
    qint64 size;
    float score;
    bool isDirectory;
    QVariantMap customAttributes;
    
    // 虚函数用于运行时类型识别
    virtual SearchResultData* clone() const;
};

/**
 * @brief 文件名搜索结果的数据类
 */
class FileNameSearchResultData : public SearchResultData
{
public:
    FileNameSearchResultData();
    FileNameSearchResultData(const QString &path);
    FileNameSearchResultData(const FileNameSearchResultData &other);
    
    // 文件名搜索特有字段
    QString matchType;
    
    // 实现克隆方法
    SearchResultData* clone() const override;
};

/**
 * @brief 内容搜索结果的数据类
 */
class ContentSearchResultData : public SearchResultData
{
public:
    ContentSearchResultData();
    ContentSearchResultData(const QString &path);
    ContentSearchResultData(const ContentSearchResultData &other);
    
    // 内容搜索特有字段
    QString highlightedContent;
    int lineNumber = -1;
    int matchStart = -1;
    int matchLength = 0;
    
    // 实现克隆方法
    SearchResultData* clone() const override;
};

/**
 * @brief 桌面应用搜索结果的数据类
 */
class DesktopSearchResultData : public SearchResultData
{
public:
    DesktopSearchResultData();
    DesktopSearchResultData(const QString &path);
    DesktopSearchResultData(const DesktopSearchResultData &other);
    
    // 桌面应用搜索特有字段
    QString applicationName;
    QString description;
    QString icon;
    QString exec;
    QStringList categories;
    bool isVisible = true;
    
    // 实现克隆方法
    SearchResultData* clone() const override;
};

}  // namespace Search
}  // namespace DFM6

#endif // SEARCH_RESULT_DATA_H 