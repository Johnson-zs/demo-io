#include <dfm6-search/searchresult.h>
#include "searchresultdata.h"
#include <QFileInfo>

namespace DFM6 {
namespace Search {

SearchResultData::SearchResultData()
    : size(0),
      score(0.0f),
      isDirectory(false)
{
}

SearchResultData::SearchResultData(const QString &path)
    : path(path),
      size(0),
      score(0.0f),
      isDirectory(false)
{
}

SearchResultData::SearchResultData(const SearchResultData &other)
    : path(other.path),
      modifiedTime(other.modifiedTime),
      size(other.size),
      highlightedContent(other.highlightedContent),
      score(other.score),
      isDirectory(other.isDirectory),
      customAttributes(other.customAttributes)
{
}

SearchResultData::SearchResultData(SearchResultData &&other) noexcept
    : path(std::move(other.path)),
      modifiedTime(std::move(other.modifiedTime)),
      size(other.size),
      score(other.score),
      isDirectory(other.isDirectory),
      customAttributes(std::move(other.customAttributes))
{
    // 清空其他对象的状态
    other.size = 0;
    other.score = 0.0f;
    other.isDirectory = false;
}

SearchResultData& SearchResultData::operator=(SearchResultData &&other) noexcept
{
    if (this != &other) {
        path = std::move(other.path);
        modifiedTime = std::move(other.modifiedTime);
        size = other.size;
        score = other.score;
        isDirectory = other.isDirectory;
        customAttributes = std::move(other.customAttributes);
        
        // 清空其他对象的状态
        other.size = 0;
        other.score = 0.0f;
        other.isDirectory = false;
    }
    return *this;
}

SearchResult::SearchResult()
    : d(std::make_unique<SearchResultData>())
{
}

SearchResult::SearchResult(const QString &path)
    : d(std::make_unique<SearchResultData>(path))
{
}

SearchResult::SearchResult(const SearchResult &other)
    : d(std::make_unique<SearchResultData>(*other.d))
{
}

SearchResult::SearchResult(SearchResult &&other) noexcept
    : d(std::move(other.d))
{
}

SearchResult::~SearchResult() = default;

SearchResult& SearchResult::operator=(const SearchResult &other)
{
    if (this != &other) {
        d = std::make_unique<SearchResultData>(*other.d);
    }
    return *this;
}

SearchResult& SearchResult::operator=(SearchResult &&other) noexcept
{
    if (this != &other) {
        d = std::move(other.d);
    }
    return *this;
}

QString SearchResult::path() const
{
    return d->path;
}

void SearchResult::setPath(const QString &path)
{
    d->path = path;
}

QString SearchResult::fileName() const
{
    QFileInfo fileInfo(d->path);
    return fileInfo.fileName();
}

QDateTime SearchResult::modifiedTime() const
{
    return d->modifiedTime;
}

void SearchResult::setModifiedTime(const QDateTime &time)
{
    d->modifiedTime = time;
}

qint64 SearchResult::size() const
{
    return d->size;
}

void SearchResult::setSize(qint64 size)
{
    d->size = size;
}

QString SearchResult::highlightedContent() const
{
    return d->highlightedContent;
}

void SearchResult::setHighlightedContent(const QString &content)
{
    d->highlightedContent = content;
}

float SearchResult::score() const
{
    return d->score;
}

void SearchResult::setScore(float score)
{
    d->score = score;
}

bool SearchResult::isDirectory() const
{
    return d->isDirectory;
}

void SearchResult::setIsDirectory(bool isDir)
{
    d->isDirectory = isDir;
}

void SearchResult::setCustomAttribute(const QString &key, const QVariant &value)
{
    d->customAttributes[key] = value;
}

QVariant SearchResult::customAttribute(const QString &key) const
{
    return d->customAttributes.value(key);
}

QVariantMap SearchResult::customAttributes() const
{
    return d->customAttributes;
}

}  // namespace Search
}  // namespace DFM6 