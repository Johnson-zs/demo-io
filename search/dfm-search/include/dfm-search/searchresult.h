#ifndef DFM_SEARCH_RESULT_H
#define DFM_SEARCH_RESULT_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QMap>
#include <QFileInfo>
#include <QVector>
#include <QPixmap>
#include <QIcon>
#include <memory>
#include <variant>

namespace DFM {
namespace Search {

/**
 * @brief 匹配段高亮信息
 */
struct MatchHighlight {
    int startPos = 0;            // 起始位置
    int length = 0;              // 长度
    QString matchedText;         // 匹配文本
    bool isTitleMatch = false;   // 是否在标题中匹配
};

/**
 * @brief 搜索结果基类
 */
class SearchResultItem {
public:
    enum class ResultType {
        File,           // 文件结果
        Content,        // 内容结果
        Application,    // 应用程序结果
        Image,          // 图像结果
        Custom          // 自定义结果
    };

    SearchResultItem() = default;
    virtual ~SearchResultItem() = default;

    virtual ResultType type() const = 0;
    
    // 通用属性
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QIcon icon() const { return m_icon; }
    double relevance() const { return m_relevance; }
    
    // 可自定义的元数据
    QVariant metadata(const QString& key) const { 
        return m_metadata.value(key); 
    }
    
    QMap<QString, QVariant> allMetadata() const { 
        return m_metadata; 
    }
    
    // 高亮信息
    QVector<MatchHighlight> highlights() const { 
        return m_highlights; 
    }
    
    // 设置方法
    void setId(const QString& id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setIcon(const QIcon& icon) { m_icon = icon; }
    void setRelevance(double relevance) { m_relevance = relevance; }
    void setMetadata(const QString& key, const QVariant& value) { 
        m_metadata[key] = value; 
    }
    void addHighlight(const MatchHighlight& highlight) { 
        m_highlights.append(highlight); 
    }

protected:
    QString m_id;
    QString m_title;
    QString m_description;
    QIcon m_icon;
    double m_relevance = 0.0;
    QMap<QString, QVariant> m_metadata;
    QVector<MatchHighlight> m_highlights;
};

/**
 * @brief 文件搜索结果
 */
class FileResultItem : public SearchResultItem {
public:
    FileResultItem() = default;
    explicit FileResultItem(const QFileInfo& fileInfo);
    
    ResultType type() const override { return ResultType::File; }
    
    QString path() const { return m_path; }
    QString mimeType() const { return m_mimeType; }
    qint64 size() const { return m_size; }
    QDateTime modifiedTime() const { return m_modifiedTime; }
    QDateTime createdTime() const { return m_createdTime; }
    bool isDir() const { return m_isDir; }
    
    void setPath(const QString& path) { m_path = path; }
    void setMimeType(const QString& mimeType) { m_mimeType = mimeType; }
    void setSize(qint64 size) { m_size = size; }
    void setModifiedTime(const QDateTime& time) { m_modifiedTime = time; }
    void setCreatedTime(const QDateTime& time) { m_createdTime = time; }
    void setIsDir(bool isDir) { m_isDir = isDir; }

private:
    QString m_path;
    QString m_mimeType;
    qint64 m_size = 0;
    QDateTime m_modifiedTime;
    QDateTime m_createdTime;
    bool m_isDir = false;
};

/**
 * @brief 内容搜索结果
 */
class ContentResultItem : public FileResultItem {
public:
    ContentResultItem() = default;
    ContentResultItem(const QFileInfo& fileInfo, const QString& content);
    
    ResultType type() const override { return ResultType::Content; }
    
    QString content() const { return m_content; }
    int lineNumber() const { return m_lineNumber; }
    int columnNumber() const { return m_columnNumber; }
    
    void setContent(const QString& content) { m_content = content; }
    void setLineNumber(int line) { m_lineNumber = line; }
    void setColumnNumber(int column) { m_columnNumber = column; }

private:
    QString m_content;
    int m_lineNumber = -1;
    int m_columnNumber = -1;
};

/**
 * @brief 应用程序搜索结果
 */
class ApplicationResultItem : public SearchResultItem {
public:
    ApplicationResultItem() = default;
    
    ResultType type() const override { return ResultType::Application; }
    
    QString execPath() const { return m_execPath; }
    QString desktopFile() const { return m_desktopFile; }
    QStringList categories() const { return m_categories; }
    
    void setExecPath(const QString& path) { m_execPath = path; }
    void setDesktopFile(const QString& desktopFile) { m_desktopFile = desktopFile; }
    void setCategories(const QStringList& categories) { m_categories = categories; }

private:
    QString m_execPath;
    QString m_desktopFile;
    QStringList m_categories;
};

/**
 * @brief 图像 OCR 搜索结果
 */
class ImageResultItem : public FileResultItem {
public:
    ImageResultItem() = default;
    
    ResultType type() const override { return ResultType::Image; }
    
    QString ocrText() const { return m_ocrText; }
    QPixmap thumbnail() const { return m_thumbnail; }
    
    void setOcrText(const QString& text) { m_ocrText = text; }
    void setThumbnail(const QPixmap& thumbnail) { m_thumbnail = thumbnail; }

private:
    QString m_ocrText;
    QPixmap m_thumbnail;
};

// 类型擦除的通用结果容器
using SearchResult = std::shared_ptr<SearchResultItem>;

/**
 * @brief 搜索结果集合
 */
class SearchResultSet {
public:
    SearchResultSet() = default;
    
    void addResult(const SearchResult& result) {
        m_results.append(result);
    }
    
    int count() const { return m_results.size(); }
    
    bool isEmpty() const { return m_results.isEmpty(); }
    
    SearchResult at(int index) const {
        return index >= 0 && index < m_results.size() ? m_results.at(index) : nullptr;
    }
    
    // 提供迭代器支持
    QVector<SearchResult>::const_iterator begin() const { return m_results.begin(); }
    QVector<SearchResult>::const_iterator end() const { return m_results.end(); }
    
    // 批量添加结果
    void merge(const SearchResultSet& other) {
        m_results.append(other.m_results);
    }
    
    // 清除所有结果
    void clear() {
        m_results.clear();
    }
    
    // 排序结果
    void sortBy(SortBy sortBy, SortOrder order = SortOrder::Ascending);
    
    // 过滤结果
    SearchResultSet filter(const std::function<bool(const SearchResult&)>& predicate) const;

private:
    QVector<SearchResult> m_results;
};

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_RESULT_H 