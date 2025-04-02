#include <dfm6-search/contentsearchapi.h>

namespace DFM6 {
namespace Search {

ContentSearchAPI::ContentSearchAPI(SearchOptions& options)
    : m_options(options)
{
}

void ContentSearchAPI::setFileTypeFilters(const QStringList &extensions)
{
    m_options.setCustomOption("fileTypeFilters", extensions);
}

QStringList ContentSearchAPI::fileTypeFilters() const
{
    return m_options.customOption("fileTypeFilters").toStringList();
}

void ContentSearchAPI::setMaxPreviewLength(int length)
{
    m_options.setCustomOption("maxPreviewLength", length);
}

int ContentSearchAPI::maxPreviewLength() const
{
    return m_options.customOption("maxPreviewLength").toInt();
}

void ContentSearchAPI::setSearchBinaryFiles(bool enabled)
{
    m_options.setCustomOption("searchBinaryFiles", enabled);
}

bool ContentSearchAPI::searchBinaryFiles() const
{
    return m_options.customOption("searchBinaryFiles").toBool();
}

void ContentSearchAPI::setIndexPath(const QString &path)
{
    m_options.setCustomOption("indexPath", path);
}

QString ContentSearchAPI::indexPath() const
{
    return m_options.customOption("indexPath").toString();
}

}  // namespace Search
}  // namespace DFM6 