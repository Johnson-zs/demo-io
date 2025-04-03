#include <dfm6-search/desktopsearchapi.h>

namespace DFM6 {
namespace Search {

DesktopSearchAPI::DesktopSearchAPI(SearchOptions& options)
    : m_options(options)
{
}

void DesktopSearchAPI::setSearchName(bool enabled)
{
    m_options.setCustomOption("searchName", enabled);
}

bool DesktopSearchAPI::searchName() const
{
    return m_options.customOption("searchName").toBool();
}

void DesktopSearchAPI::setSearchDescription(bool enabled)
{
    m_options.setCustomOption("searchDescription", enabled);
}

bool DesktopSearchAPI::searchDescription() const
{
    return m_options.customOption("searchDescription").toBool();
}

void DesktopSearchAPI::setSearchKeywords(bool enabled)
{
    m_options.setCustomOption("searchKeywords", enabled);
}

bool DesktopSearchAPI::searchKeywords() const
{
    return m_options.customOption("searchKeywords").toBool();
}

void DesktopSearchAPI::setOnlyShowVisible(bool enabled)
{
    m_options.setCustomOption("onlyShowVisible", enabled);
}

bool DesktopSearchAPI::onlyShowVisible() const
{
    return m_options.customOption("onlyShowVisible").toBool();
}

void DesktopSearchAPI::setCategories(const QStringList &categories)
{
    m_options.setCustomOption("categories", categories);
}

QStringList DesktopSearchAPI::categories() const
{
    return m_options.customOption("categories").toStringList();
}

void DesktopSearchAPI::setSortResults(bool enabled)
{
    m_options.setCustomOption("sortResults", enabled);
}

bool DesktopSearchAPI::sortResults() const
{
    return m_options.customOption("sortResults").toBool();
}

}  // namespace Search
}  // namespace DFM6 