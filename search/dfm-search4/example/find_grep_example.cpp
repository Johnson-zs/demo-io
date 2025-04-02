#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchfactory.h>

using namespace DFM6::Search;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    
    parser.addOption({"path", "搜索路径", "path", "."});
    parser.addOption({"name", "文件名模式 (类似find -name)", "pattern"});
    parser.addOption({"content", "内容模式 (类似grep)", "pattern"});
    parser.process(app);
    
    QString path = parser.value("path");
    QString namePattern = parser.value("name");
    QString contentPattern = parser.value("content");
    
    if (namePattern.isEmpty() && contentPattern.isEmpty()) {
        qInfo() << "请指定 --name 或 --content 参数";
        return 1;
    }
    
    // 创建查询
    SearchQuery query;
    if (!namePattern.isEmpty()) {
        query = SearchQuery::createWildcardQuery(namePattern);
    } else {
        query = SearchQuery::createSimpleQuery(contentPattern);
    }
    
    // 开始实时搜索
    qInfo() << "开始搜索...";
    QElapsedTimer timer;
    timer.start();
    
    // 使用工厂模式创建搜索引擎
    std::shared_ptr<SearchEngine> engine;
    if (!contentPattern.isEmpty()) {
        engine = SearchFactory::createEngine(SearchType::Content);
    } else {
        engine = SearchFactory::createEngine(SearchType::FileName);
    }
    
    // 使用新的结果类型处理
    engine->searchWithCallback(query, [&timer](const SearchResultBase& result) {
        // 根据结果类型处理
        if (result.resultType() == SearchType::FileName) {
            const FileNameSearchResult* fileResult = static_cast<const FileNameSearchResult*>(&result);
            qInfo().noquote() << fileResult->path();
        } else if (result.resultType() == SearchType::Content) {
            const ContentSearchResult* contentResult = static_cast<const ContentSearchResult*>(&result);
            qInfo().noquote() << contentResult->path() << ":" 
                             << contentResult->highlightedContent().trimmed();
        }
        return true; // 继续搜索
    });
    
    // 搜索结束处理
    QObject::connect(&engine, &SearchEngine::searchFinished, 
                    [&timer, &app](const QList<SearchResult>& results) {
        qInfo() << "搜索完成，耗时" << timer.elapsed() / 1000.0 << "秒";
        app.quit();
    });
    
    return app.exec();
} 