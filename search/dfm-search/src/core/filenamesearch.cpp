#include "dfm-search/filenamesearch.h"
#include <QDirIterator>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QFile>
#include <QDataStream>
#include <QFileIconProvider>
#include <QMimeDatabase>
#include <QtConcurrent>

namespace DFM {
namespace Search {

//---------------------------------------------------------------------
// RealtimeFilenameSearchEngine 实现
//---------------------------------------------------------------------

RealtimeFilenameSearchEngine::RealtimeFilenameSearchEngine(QObject* parent)
    : BaseSearchEngine(parent)
{
}

RealtimeFilenameSearchEngine::~RealtimeFilenameSearchEngine()
{
}

bool RealtimeFilenameSearchEngine::prepare()
{
    return true;
}

bool RealtimeFilenameSearchEngine::startSearch(const QString& query)
{
    if (query.isEmpty()) {
        setError("搜索关键词不能为空");
        return false;
    }
    
    // 创建一个简单的结果集
    SearchResultSet results;
    FileResultItem item;
    item.setTitle("测试结果 - " + query);
    item.setPath("/tmp/test.txt");
    results.addResult(std::make_shared<FileResultItem>(item));
    
    addResults(results);
    setState(SearchState::Completed);
    return true;
}

QString RealtimeFilenameSearchEngine::name() const
{
    return "Realtime Filename Search";
}

QString RealtimeFilenameSearchEngine::description() const
{
    return "实时文件名搜索引擎";
}

SearchType RealtimeFilenameSearchEngine::supportedType() const
{
    return SearchType::FileName;
}

SearchMode RealtimeFilenameSearchEngine::supportedMode() const
{
    return SearchMode::Realtime;
}

bool RealtimeFilenameSearchEngine::hasCapability(const QString& capability) const
{
    return false;
}

//---------------------------------------------------------------------
// IndexedFilenameSearchEngine 实现
//---------------------------------------------------------------------

IndexedFilenameSearchEngine::IndexedFilenameSearchEngine(QObject* parent)
    : BaseSearchEngine(parent)
{
}

IndexedFilenameSearchEngine::~IndexedFilenameSearchEngine()
{
}

bool IndexedFilenameSearchEngine::prepare()
{
    return true;
}

bool IndexedFilenameSearchEngine::startSearch(const QString& query)
{
    if (query.isEmpty()) {
        setError("搜索关键词不能为空");
        return false;
    }
    
    // 创建一个简单的结果集
    SearchResultSet results;
    FileResultItem item;
    item.setTitle("测试索引结果 - " + query);
    item.setPath("/tmp/test-indexed.txt");
    results.addResult(std::make_shared<FileResultItem>(item));
    
    addResults(results);
    setState(SearchState::Completed);
    return true;
}

bool IndexedFilenameSearchEngine::rebuildIndex()
{
    return true;
}

QString IndexedFilenameSearchEngine::name() const
{
    return "Indexed Filename Search";
}

QString IndexedFilenameSearchEngine::description() const
{
    return "索引文件名搜索引擎";
}

SearchType IndexedFilenameSearchEngine::supportedType() const
{
    return SearchType::FileName;
}

SearchMode IndexedFilenameSearchEngine::supportedMode() const
{
    return SearchMode::Indexed;
}

bool IndexedFilenameSearchEngine::hasCapability(const QString& capability) const
{
    return false;
}

} // namespace Search
} // namespace DFM 
