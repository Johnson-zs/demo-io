#ifndef DFM_SEARCH_FILTER_H
#define DFM_SEARCH_FILTER_H

#include <memory>
#include <vector>
#include <functional>

#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QFileInfo>

#include "search_engine.h" // 引入SearchOptions和相关结构

namespace DFM {
namespace Search {

/**
 * @brief 过滤器接口基类
 * 
 * 所有过滤器必须实现此接口
 */
class Filter {
public:
    virtual ~Filter() = default;
    
    /**
     * @brief 判断文件是否匹配过滤器
     * @param fileInfo 文件信息
     * @return 是否匹配
     */
    virtual bool matches(const QFileInfo& fileInfo) const = 0;
    
    /**
     * @brief 获取过滤器的描述信息
     * @return 过滤器描述
     */
    virtual QString description() const = 0;
};

/**
 * @brief 文件名过滤器
 */
class FilenameFilter : public Filter {
public:
    FilenameFilter(const QString& pattern, bool caseSensitive = false);
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    QString pattern_;
    bool caseSensitive_;
};

/**
 * @brief 文件扩展名过滤器
 */
class ExtensionFilter : public Filter {
public:
    ExtensionFilter(const QStringList& extensions);
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    QStringList extensions_;
};

/**
 * @brief 文件大小过滤器
 */
class SizeFilter : public Filter {
public:
    SizeFilter(qint64 minSize, qint64 maxSize = -1);
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    qint64 minSize_;
    qint64 maxSize_;
};

/**
 * @brief 文件修改时间过滤器
 */
class DateFilter : public Filter {
public:
    DateFilter(const QDateTime& fromDate, const QDateTime& toDate = QDateTime());
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    QDateTime fromDate_;
    QDateTime toDate_;
};

/**
 * @brief 路径过滤器
 */
class PathFilter : public Filter {
public:
    PathFilter(const QStringList& includePaths, const QStringList& excludePaths = {});
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    QStringList includePaths_;
    QStringList excludePaths_;
};

/**
 * @brief 过滤器组（复合过滤器）
 * 
 * 用于组合多个过滤器，可以是AND或OR关系
 */
class FilterGroup : public Filter {
public:
    enum class Operation {
        AND,
        OR
    };
    
    FilterGroup(Operation op = Operation::AND);
    
    void addFilter(std::shared_ptr<Filter> filter);
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    std::vector<std::shared_ptr<Filter>> filters_;
    Operation operation_;
};

/**
 * @brief 自定义过滤器
 * 
 * 使用回调函数实现的过滤器
 */
class CustomFilter : public Filter {
public:
    using MatchFunction = std::function<bool(const QFileInfo&)>;
    
    CustomFilter(MatchFunction matchFunc, const QString& description = "自定义过滤器");
    bool matches(const QFileInfo& fileInfo) const override;
    QString description() const override;
    
private:
    MatchFunction matchFunc_;
    QString description_;
};

/**
 * @brief 过滤器工厂
 * 
 * 用于创建和组合各种过滤器
 */
class FilterFactory {
public:
    static std::shared_ptr<Filter> createFromOptions(const SearchOptions& options);
    
    static std::shared_ptr<FilenameFilter> createFilenameFilter(const QString& pattern, bool caseSensitive = false);
    static std::shared_ptr<ExtensionFilter> createExtensionFilter(const QStringList& extensions);
    static std::shared_ptr<SizeFilter> createSizeFilter(qint64 minSize, qint64 maxSize = -1);
    static std::shared_ptr<DateFilter> createDateFilter(const QDateTime& fromDate, const QDateTime& toDate = QDateTime());
    static std::shared_ptr<PathFilter> createPathFilter(const QStringList& includePaths, const QStringList& excludePaths = {});
    static std::shared_ptr<FilterGroup> createFilterGroup(FilterGroup::Operation op = FilterGroup::Operation::AND);
    static std::shared_ptr<CustomFilter> createCustomFilter(CustomFilter::MatchFunction matchFunc, const QString& description = "自定义过滤器");
};

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_FILTER_H 