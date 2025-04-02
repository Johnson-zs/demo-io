#include "contentsearchoptions.h"

namespace DFM6 {
namespace Search {

ContentSearchOptions::ContentSearchOptions()
    : SearchOptions()
{
    // 设置内容搜索的特定默认选项
    setCustomOption("maxPreviewLength", 200);
    setCustomOption("searchBinaryFiles", false);
}

ContentSearchOptions::ContentSearchOptions(const ContentSearchOptions &other)
    : SearchOptions(other)
{
    // 自定义选项已通过基类复制
}

ContentSearchOptions::ContentSearchOptions(ContentSearchOptions &&other) noexcept
    : SearchOptions(std::move(other))
{
    // 自定义选项已通过基类移动
}

ContentSearchOptions::ContentSearchOptions(const SearchOptions &options)
    : SearchOptions(options)
{
    // 如果基类选项没有设置特定选项，设置默认值
    if (!hasCustomOption("maxPreviewLength")) {
        setCustomOption("maxPreviewLength", 200);
    }
    if (!hasCustomOption("searchBinaryFiles")) {
        setCustomOption("searchBinaryFiles", false);
    }
}

ContentSearchOptions::~ContentSearchOptions() = default;

ContentSearchOptions& ContentSearchOptions::operator=(const ContentSearchOptions &other)
{
    if (this != &other) {
        SearchOptions::operator=(other);
    }
    return *this;
}

ContentSearchOptions& ContentSearchOptions::operator=(ContentSearchOptions &&other) noexcept
{
    if (this != &other) {
        SearchOptions::operator=(std::move(other));
    }
    return *this;
}

void ContentSearchOptions::setFileTypeFilters(const QStringList &extensions)
{
    setCustomOption("fileTypeFilters", extensions);
}

QStringList ContentSearchOptions::fileTypeFilters() const
{
    return customOption("fileTypeFilters").toStringList();
}

void ContentSearchOptions::setMaxPreviewLength(int length)
{
    setCustomOption("maxPreviewLength", length);
}

int ContentSearchOptions::maxPreviewLength() const
{
    return customOption("maxPreviewLength").toInt();
}

void ContentSearchOptions::setSearchBinaryFiles(bool enabled)
{
    setCustomOption("searchBinaryFiles", enabled);
}

bool ContentSearchOptions::searchBinaryFiles() const
{
    return customOption("searchBinaryFiles").toBool();
}

void ContentSearchOptions::setIndexPath(const QString &path)
{
    setCustomOption("indexPath", path);
}

QString ContentSearchOptions::indexPath() const
{
    return customOption("indexPath").toString();
}

std::unique_ptr<SearchOptions> ContentSearchOptions::clone() const
{
    return std::make_unique<ContentSearchOptions>(*this);
}

}  // namespace Search
}  // namespace DFM6 