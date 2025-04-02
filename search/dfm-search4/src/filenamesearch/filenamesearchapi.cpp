#include <dfm6-search/filenamesearchapi.h>

namespace DFM6 {
namespace Search {

FileNameSearchAPI::FileNameSearchAPI(SearchOptions& options)
    : m_options(options)
{
}

void FileNameSearchAPI::setPinyinEnabled(bool enabled)
{
    m_options.setCustomOption("pinyinEnabled", enabled);
}

bool FileNameSearchAPI::pinyinEnabled() const
{
    return m_options.customOption("pinyinEnabled").toBool();
}

void FileNameSearchAPI::setFuzzySearch(bool enabled)
{
    m_options.setCustomOption("fuzzySearch", enabled);
}

bool FileNameSearchAPI::fuzzySearch() const
{
    return m_options.customOption("fuzzySearch").toBool();
}

void FileNameSearchAPI::setFileTypes(const QStringList &types)
{
    m_options.setCustomOption("fileTypes", types);
}

QStringList FileNameSearchAPI::fileTypes() const
{
    return m_options.customOption("fileTypes").toStringList();
}

// 其他方法实现...

}  // namespace Search
}  // namespace DFM6 