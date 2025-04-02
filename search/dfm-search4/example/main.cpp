#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFileInfo>

#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchfactory.h>

using namespace DFM6::Search;

void printResult(const SearchResult &result)
{
    qInfo() << "Found:" << result.path();
    if (!result.highlightedContent().isEmpty()) {
        qInfo() << "  Content:" << result.highlightedContent();
    }
    qInfo() << "  Size:" << result.size() << "bytes";
    qInfo() << "  Modified:" << result.modifiedTime().toString();
    qInfo() << "-----------------------------------";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("dfm6-search-example");
    app.setApplicationVersion("1.0");
    
    // 命令行解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM6 Search Library Example");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 搜索类型选项
    QCommandLineOption typeOption(QStringList() << "t" << "type",
                                 "Search type (filename, content)",
                                 "type", "filename");
    parser.addOption(typeOption);
    
    // 关键词选项
    QCommandLineOption queryOption(QStringList() << "q" << "query",
                                  "Search query",
                                  "query");
    parser.addOption(queryOption);
    
    // 路径选项
    QCommandLineOption pathOption(QStringList() << "p" << "path",
                                 "Search path",
                                 "path", QDir::currentPath());
    parser.addOption(pathOption);
    
    // 区分大小写选项
    QCommandLineOption caseOption(QStringList() << "c" << "case-sensitive",
                                 "Case sensitive search");
    parser.addOption(caseOption);
    
    // 解析命令行
    parser.process(app);
    
    // 获取搜索类型
    SearchType searchType = SearchType::FileName;
    QString typeStr = parser.value(typeOption).toLower();
    if (typeStr == "content") {
        searchType = SearchType::Content;
    }
    
    // 检查必须的参数
    if (!parser.isSet(queryOption)) {
        qCritical() << "Error: Query parameter is required.";
        parser.showHelp(1);
        return 1;
    }
    
    QString query = parser.value(queryOption);
    QString path = parser.value(pathOption);
    bool caseSensitive = parser.isSet(caseOption);
    
    // 创建搜索引擎
    SearchEngine engine(searchType);
    
    // 配置搜索选项
    SearchOptions options = engine.searchOptions();
    options.setSearchPath(path);
    options.setCaseSensitive(caseSensitive);
    engine.setSearchOptions(options);
    
    // 创建查询
    SearchQuery searchQuery = SearchQuery::createSimpleQuery(query);
    
    // 连接信号
    QObject::connect(&engine, &SearchEngine::searchStarted, []() {
        qInfo() << "Search started...";
    });
    
    QObject::connect(&engine, &SearchEngine::resultFound, &printResult);
    
    QObject::connect(&engine, &SearchEngine::progressChanged, [](int current, int total) {
        qInfo() << "Progress:" << current << "/" << total;
    });
    
    QObject::connect(&engine, &SearchEngine::searchFinished, [&app](const QList<SearchResult> &results) {
        qInfo() << "Search completed with" << results.size() << "results.";
        app.quit();
    });
    
    QObject::connect(&engine, &SearchEngine::error, [](const QString &message) {
        qCritical() << "Error:" << message;
    });
    
    // 执行搜索
    qInfo() << "Searching for" << query << "in" << path;
    engine.search(searchQuery);
    
    return app.exec();
} 