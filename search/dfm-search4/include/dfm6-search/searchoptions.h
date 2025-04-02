#ifndef DFM6_SEARCH_OPTIONS_H
#define DFM6_SEARCH_OPTIONS_H

#include <dfm6-search/global.h>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <memory>

namespace DFM6 {
namespace Search {

class SearchOptionsData;

/**
 * @brief 搜索选项基类
 *
 * 定义通用的搜索选项
 */
class DFM6_SEARCH_EXPORT SearchOptions
{
public:
    /**
     * @brief 构造函数
     */
    SearchOptions();

    /**
     * @brief 拷贝构造函数
     */
    SearchOptions(const SearchOptions &other);

    /**
     * @brief 移动构造函数
     */
    SearchOptions(SearchOptions &&other) noexcept;

    /**
     * @brief 析构函数
     */
    virtual ~SearchOptions();

    /**
     * @brief 赋值操作符
     */
    SearchOptions &operator=(const SearchOptions &other);

    /**
     * @brief 移动赋值操作符
     */
    SearchOptions &operator=(SearchOptions &&other) noexcept;

    /**
     * @brief 获取搜索方法
     */
    SearchMethod method() const;

    /**
     * @brief 设置搜索方法
     * 索引搜索适合已建立索引的目录，速度快但可能不是最新结果
     * 实时搜索直接扫描文件系统，结果是最新的但速度可能较慢
     */
    void setSearchMethod(SearchMethod method);

    /**
     * @brief 判断是否区分大小写
     */
    bool caseSensitive() const;

    /**
     * @brief 设置是否区分大小写
     */
    void setCaseSensitive(bool sensitive);

    /**
     * @brief 获取起始搜索路径
     */
    QString searchPath() const;

    /**
     * @brief 设置起始搜索路径
     */
    void setSearchPath(const QString &path);

    /**
     * @brief 获取排除路径列表
     */
    QStringList excludePaths() const;

    /**
     * @brief 设置排除路径列表
     */
    void setExcludePaths(const QStringList &paths);

    /**
     * @brief 添加排除路径
     */
    void addExcludePath(const QString &path);

    /**
     * @brief 设置是否包含隐藏文件
     */
    void setIncludeHidden(bool include);

    /**
     * @brief 是否包含隐藏文件
     */
    bool includeHidden() const;

    /**
     * @brief 获取最大结果数量限制
     */
    int maxResults() const;

    /**
     * @brief 设置最大结果数量限制
     */
    void setMaxResults(int count);

    /**
     * @brief 设置自定义选项
     */
    void setCustomOption(const QString &key, const QVariant &value);

    /**
     * @brief 获取自定义选项
     */
    QVariant customOption(const QString &key) const;

    /**
     * @brief 判断是否设置了指定的自定义选项
     */
    bool hasCustomOption(const QString &key) const;

    /**
     * @brief 获取自定义选项映射
     */
    QVariantMap customOptions() const;

    /**
     * @brief 克隆当前选项
     */
    virtual std::unique_ptr<SearchOptions> clone() const;

    /**
     * @brief 设置是否递归搜索
     * @param recursive 是否递归搜索子目录
     */
    void setRecursive(bool recursive);

    /**
     * @brief 获取是否递归搜索
     * @return 是否递归搜索子目录
     */
    bool isRecursive() const;

protected:
    // 访问私有数据的辅助方法
    SearchOptionsData *data();
    const SearchOptionsData *data() const;

private:
    std::unique_ptr<SearchOptionsData> d;   // PIMPL
};

}   // namespace Search
}   // namespace DFM6

#endif   // DFM6_SEARCH_OPTIONS_H
