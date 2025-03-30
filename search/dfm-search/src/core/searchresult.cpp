#include "dfm-search/searchengine.h"

namespace DFM {
namespace Search {

//---------------------------------------------------------------------
// SearchResultSet 实现
//---------------------------------------------------------------------

void SearchResultSet::addResult(const std::shared_ptr<FileResultItem>& result) {
    if (result) {
        m_results.push_back(result);
    }
}

const std::vector<std::shared_ptr<ISearchResult>>& SearchResultSet::results() const {
    return m_results;
}

int SearchResultSet::count() const {
    return static_cast<int>(m_results.size());
}

bool SearchResultSet::isEmpty() const {
    return m_results.empty();
}

void SearchResultSet::clear() {
    m_results.clear();
}

//---------------------------------------------------------------------
// FileResultItem 实现
//---------------------------------------------------------------------

FileResultItem::FileResultItem()
    : m_size(0)
    , m_isDir(false)
    , m_relevance(0.0)
{
}

FileResultItem::~FileResultItem() = default;

QString FileResultItem::id() const {
    return m_id;
}

void FileResultItem::setId(const QString& id) {
    m_id = id;
}

QString FileResultItem::title() const {
    return m_title;
}

void FileResultItem::setTitle(const QString& title) {
    m_title = title;
}

QString FileResultItem::description() const {
    return m_description;
}

void FileResultItem::setDescription(const QString& description) {
    m_description = description;
}

QIcon FileResultItem::icon() const {
    return m_icon;
}

void FileResultItem::setIcon(const QIcon& icon) {
    m_icon = icon;
}

double FileResultItem::relevance() const {
    return m_relevance;
}

void FileResultItem::setRelevance(double relevance) {
    m_relevance = relevance;
}

QString FileResultItem::path() const {
    return m_path;
}

void FileResultItem::setPath(const QString& path) {
    m_path = path;
}

qint64 FileResultItem::size() const {
    return m_size;
}

void FileResultItem::setSize(qint64 size) {
    m_size = size;
}

QDateTime FileResultItem::modifiedTime() const {
    return m_modifiedTime;
}

void FileResultItem::setModifiedTime(const QDateTime& time) {
    m_modifiedTime = time;
}

QDateTime FileResultItem::createdTime() const {
    return m_createdTime;
}

void FileResultItem::setCreatedTime(const QDateTime& time) {
    m_createdTime = time;
}

bool FileResultItem::isDir() const {
    return m_isDir;
}

void FileResultItem::setIsDir(bool isDir) {
    m_isDir = isDir;
}

QString FileResultItem::mimeType() const {
    return m_mimeType;
}

void FileResultItem::setMimeType(const QString& mimeType) {
    m_mimeType = mimeType;
}

const QVector<MatchHighlight>& FileResultItem::highlights() const {
    return m_highlights;
}

void FileResultItem::addHighlight(const MatchHighlight& highlight) {
    m_highlights.append(highlight);
}

void FileResultItem::clearHighlights() {
    m_highlights.clear();
}

//---------------------------------------------------------------------
// ContentResultItem 实现
//---------------------------------------------------------------------

ContentResultItem::ContentResultItem()
    : m_size(0)
    , m_isDir(false)
    , m_relevance(0.0)
    , m_lineNumber(-1)
    , m_columnNumber(-1)
{
}

ContentResultItem::~ContentResultItem() = default;

QString ContentResultItem::content() const {
    return m_content;
}

void ContentResultItem::setContent(const QString& content) {
    m_content = content;
}

int ContentResultItem::lineNumber() const {
    return m_lineNumber;
}

void ContentResultItem::setLineNumber(int lineNumber) {
    m_lineNumber = lineNumber;
}

int ContentResultItem::columnNumber() const {
    return m_columnNumber;
}

void ContentResultItem::setColumnNumber(int columnNumber) {
    m_columnNumber = columnNumber;
}

QString ContentResultItem::id() const {
    return m_id;
}

void ContentResultItem::setId(const QString& id) {
    m_id = id;
}

QString ContentResultItem::title() const {
    return m_title;
}

void ContentResultItem::setTitle(const QString& title) {
    m_title = title;
}

QString ContentResultItem::description() const {
    return m_description;
}

void ContentResultItem::setDescription(const QString& description) {
    m_description = description;
}

QIcon ContentResultItem::icon() const {
    return m_icon;
}

void ContentResultItem::setIcon(const QIcon& icon) {
    m_icon = icon;
}

double ContentResultItem::relevance() const {
    return m_relevance;
}

void ContentResultItem::setRelevance(double relevance) {
    m_relevance = relevance;
}

QString ContentResultItem::path() const {
    return m_path;
}

void ContentResultItem::setPath(const QString& path) {
    m_path = path;
}

qint64 ContentResultItem::size() const {
    return m_size;
}

void ContentResultItem::setSize(qint64 size) {
    m_size = size;
}

QDateTime ContentResultItem::modifiedTime() const {
    return m_modifiedTime;
}

void ContentResultItem::setModifiedTime(const QDateTime& time) {
    m_modifiedTime = time;
}

QDateTime ContentResultItem::createdTime() const {
    return m_createdTime;
}

void ContentResultItem::setCreatedTime(const QDateTime& time) {
    m_createdTime = time;
}

bool ContentResultItem::isDir() const {
    return m_isDir;
}

void ContentResultItem::setIsDir(bool isDir) {
    m_isDir = isDir;
}

QString ContentResultItem::mimeType() const {
    return m_mimeType;
}

void ContentResultItem::setMimeType(const QString& mimeType) {
    m_mimeType = mimeType;
}

const QVector<MatchHighlight>& ContentResultItem::highlights() const {
    return m_highlights;
}

void ContentResultItem::addHighlight(const MatchHighlight& highlight) {
    m_highlights.append(highlight);
}

void ContentResultItem::clearHighlights() {
    m_highlights.clear();
}

} // namespace Search
} // namespace DFM 