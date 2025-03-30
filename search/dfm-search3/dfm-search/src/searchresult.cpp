#include "dfm-search/searchresult.h"
#include <QMap>

namespace DFMSearch {

// 搜索结果私有实现类
class SearchResultPrivate {
public:
    ResultType type;
    QString title;
    QString path;
    QString description;
    double score = 0.0;
    QMap<QString, QVariant> extraData;
};

// ======= SearchResult 实现 =======
SearchResult::SearchResult(ResultType type)
    : d(std::make_unique<SearchResultPrivate>())
{
    d->type = type;
}

SearchResult::SearchResult(const SearchResult& other)
    : d(std::make_unique<SearchResultPrivate>(*other.d))
{
}

SearchResult::SearchResult(SearchResult&& other) noexcept
    : d(std::move(other.d))
{
}

SearchResult& SearchResult::operator=(const SearchResult& other)
{
    if (this != &other) {
        d = std::make_unique<SearchResultPrivate>(*other.d);
    }
    return *this;
}

SearchResult& SearchResult::operator=(SearchResult&& other) noexcept
{
    if (this != &other) {
        d = std::move(other.d);
    }
    return *this;
}

SearchResult::~SearchResult() = default;

ResultType SearchResult::type() const
{
    return d->type;
}

void SearchResult::setTitle(const QString& title)
{
    d->title = title;
}

QString SearchResult::title() const
{
    return d->title;
}

void SearchResult::setPath(const QString& path)
{
    d->path = path;
}

QString SearchResult::path() const
{
    return d->path;
}

void SearchResult::setDescription(const QString& description)
{
    d->description = description;
}

QString SearchResult::description() const
{
    return d->description;
}

void SearchResult::setScore(double score)
{
    d->score = score;
}

double SearchResult::score() const
{
    return d->score;
}

void SearchResult::setData(const QString& key, const QVariant& value)
{
    d->extraData[key] = value;
}

QVariant SearchResult::data(const QString& key) const
{
    return d->extraData.value(key);
}

bool SearchResult::hasData(const QString& key) const
{
    return d->extraData.contains(key);
}

QString SearchResult::toString() const
{
    return QString("[%1] %2 - %3")
            .arg(d->type == ResultType::File ? "File" : 
                 d->type == ResultType::Content ? "Content" : 
                 d->type == ResultType::Application ? "App" : "Custom")
            .arg(d->title)
            .arg(d->path);
}

// ======= FileSearchResult 实现 =======
FileSearchResult::FileSearchResult()
    : SearchResult(ResultType::File)
{
}

void FileSearchResult::setFileSize(qint64 size)
{
    setData("fileSize", size);
}

qint64 FileSearchResult::fileSize() const
{
    return data("fileSize").toLongLong();
}

void FileSearchResult::setModifiedTime(const QDateTime& time)
{
    setData("modifiedTime", time);
}

QDateTime FileSearchResult::modifiedTime() const
{
    return data("modifiedTime").toDateTime();
}

QString FileSearchResult::toString() const
{
    QString sizeStr;
    if (hasData("fileSize")) {
        const qint64 size = fileSize();
        if (size < 1024) {
            sizeStr = QString::number(size) + " B";
        } else if (size < 1024 * 1024) {
            sizeStr = QString::number(size / 1024.0, 'f', 1) + " KB";
        } else if (size < 1024 * 1024 * 1024) {
            sizeStr = QString::number(size / (1024.0 * 1024.0), 'f', 1) + " MB";
        } else {
            sizeStr = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
        }
    }
    
    return QString("文件：%1 [%2] - %3")
            .arg(title())
            .arg(sizeStr)
            .arg(path());
}

// ======= ContentSearchResult 实现 =======
ContentSearchResult::ContentSearchResult()
    : SearchResult(ResultType::Content)
{
}

void ContentSearchResult::setMatchedContent(const QString& content)
{
    setData("matchedContent", content);
}

QString ContentSearchResult::matchedContent() const
{
    return data("matchedContent").toString();
}

void ContentSearchResult::setLineNumber(int lineNumber)
{
    setData("lineNumber", lineNumber);
}

int ContentSearchResult::lineNumber() const
{
    return data("lineNumber").toInt();
}

QString ContentSearchResult::toString() const
{
    return QString("内容：%1:%2 - \"%3\"")
            .arg(path())
            .arg(hasData("lineNumber") ? QString::number(lineNumber()) : "?")
            .arg(hasData("matchedContent") ? matchedContent() : "");
}

// ======= AppSearchResult 实现 =======
AppSearchResult::AppSearchResult()
    : SearchResult(ResultType::Application)
{
}

void AppSearchResult::setIconPath(const QString& iconPath)
{
    setData("iconPath", iconPath);
}

QString AppSearchResult::iconPath() const
{
    return data("iconPath").toString();
}

void AppSearchResult::setExecuteCommand(const QString& command)
{
    setData("executeCommand", command);
}

QString AppSearchResult::executeCommand() const
{
    return data("executeCommand").toString();
}

QString AppSearchResult::toString() const
{
    return QString("应用：%1 [%2]")
            .arg(title())
            .arg(hasData("executeCommand") ? executeCommand() : "");
}

} // namespace DFMSearch 