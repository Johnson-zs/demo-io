#include "desktopsearchoptions.h"

namespace DFM6 {
namespace Search {

DesktopSearchOptions::DesktopSearchOptions()
    : SearchOptions()
{
    // 设置桌面应用搜索的特定默认选项
    setCustomOption("searchName", true);
    setCustomOption("searchDescription", true);
    setCustomOption("searchKeywords", true);
    setCustomOption("onlyShowVisible", true);
    setCustomOption("sortResults", true);
}

DesktopSearchOptions::DesktopSearchOptions(const DesktopSearchOptions &other)
    : SearchOptions(other)
{
    // 自定义选项已通过基类复制
}

DesktopSearchOptions::DesktopSearchOptions(DesktopSearchOptions &&other) noexcept
    : SearchOptions(std::move(other))
{
    // 自定义选项已通过基类移动
}

DesktopSearchOptions::DesktopSearchOptions(const SearchOptions &options)
    : SearchOptions(options)
{
    // 如果基类选项没有设置特定选项，设置默认值
    if (!hasCustomOption("searchName")) {
        setCustomOption("searchName", true);
    }
    if (!hasCustomOption("searchDescription")) {
        setCustomOption("searchDescription", true);
    }
    if (!hasCustomOption("searchKeywords")) {
        setCustomOption("searchKeywords", true);
    }
    if (!hasCustomOption("onlyShowVisible")) {
        setCustomOption("onlyShowVisible", true);
    }
    if (!hasCustomOption("sortResults")) {
        setCustomOption("sortResults", true);
    }
}

DesktopSearchOptions::~DesktopSearchOptions() = default;

DesktopSearchOptions& DesktopSearchOptions::operator=(const DesktopSearchOptions &other)
{
    if (this != &other) {
        SearchOptions::operator=(other);
    }
    return *this;
}

DesktopSearchOptions& DesktopSearchOptions::operator=(DesktopSearchOptions &&other) noexcept
{
    if (this != &other) {
        SearchOptions::operator=(std::move(other));
    }
    return *this;
}

void DesktopSearchOptions::setSearchName(bool enabled)
{
    setCustomOption("searchName", enabled);
}

bool DesktopSearchOptions::searchName() const
{
    return customOption("searchName").toBool();
}

void DesktopSearchOptions::setSearchDescription(bool enabled)
{
    setCustomOption("searchDescription", enabled);
}

bool DesktopSearchOptions::searchDescription() const
{
    return customOption("searchDescription").toBool();
}

void DesktopSearchOptions::setSearchKeywords(bool enabled)
{
    setCustomOption("searchKeywords", enabled);
}

bool DesktopSearchOptions::searchKeywords() const
{
    return customOption("searchKeywords").toBool();
}

void DesktopSearchOptions::setOnlyShowVisible(bool enabled)
{
    setCustomOption("onlyShowVisible", enabled);
}

bool DesktopSearchOptions::onlyShowVisible() const
{
    return customOption("onlyShowVisible").toBool();
}

void DesktopSearchOptions::setCategories(const QStringList &categories)
{
    setCustomOption("categories", categories);
}

QStringList DesktopSearchOptions::categories() const
{
    return customOption("categories").toStringList();
}

void DesktopSearchOptions::setSortResults(bool enabled)
{
    setCustomOption("sortResults", enabled);
}

bool DesktopSearchOptions::sortResults() const
{
    return customOption("sortResults").toBool();
}

std::unique_ptr<SearchOptions> DesktopSearchOptions::clone() const
{
    return std::make_unique<DesktopSearchOptions>(*this);
}

}  // namespace Search
}  // namespace DFM6 