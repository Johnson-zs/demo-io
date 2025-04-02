#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QStringList>
#include <QSet>
#include <QDir>

#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchfactory.h>
#include <dfm6-search/searchresult.h>
#include <dfm6-search/filenamesearchapi.h>

using namespace DFM6::Search;

// 打印文件名搜索结果的函数
void printFileNameResult(const SearchResult &result)
{
    qInfo() << "文件名搜索结果:";
    qInfo() << "  路径:" << result.path();
    qInfo() << "  大小:" << result.size() << "字节";
    qInfo() << "  修改时间:" << result.modifiedTime().toString("yyyy-MM-dd hh:mm:ss");
    qInfo() << "  匹配分数:" << result.score();

    // 获取自定义属性（如果有）
    if (result.customAttributes().contains("matchType")) {
        qInfo() << "  匹配类型:" << result.customAttribute("matchType").toString();
    }
    qInfo() << "-----------------------------------";
}

// 打印内容搜索结果的函数
void printContentResult(const SearchResult &result)
{
    qInfo() << "内容搜索结果:";
    qInfo() << "  路径:" << result.path();
    if (!result.highlightedContent().isEmpty()) {
        qInfo() << "  匹配内容:" << result.highlightedContent();
    }
    qInfo() << "  大小:" << result.size() << "字节";
    qInfo() << "  修改时间:" << result.modifiedTime().toString("yyyy-MM-dd hh:mm:ss");
    qInfo() << "-----------------------------------";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("dfm6-combined-search");
    app.setApplicationVersion("1.0");

    // 命令行解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM6 Combined Search Example");
    parser.addHelpOption();
    parser.addVersionOption();

    // 路径选项
    QCommandLineOption pathOption(QStringList() << "p"
                                                << "path",
                                  "Search path",
                                  "path", QDir::currentPath());
    parser.addOption(pathOption);

    // 关键词选项，可以指定多个
    QCommandLineOption keywordsOption(QStringList() << "k"
                                                    << "keywords",
                                      "Search keywords (can be specified multiple times)",
                                      "keyword");
    parser.addOption(keywordsOption);

    // 区分大小写选项
    QCommandLineOption caseOption(QStringList() << "c"
                                                << "case-sensitive",
                                  "Case sensitive search");
    parser.addOption(caseOption);

    // 递归搜索选项
    QCommandLineOption recursiveOption(QStringList() << "r"
                                                     << "recursive",
                                       "Search recursively");
    parser.addOption(recursiveOption);

    // 内容预览长度
    QCommandLineOption previewLengthOption(QStringList() << "l"
                                                         << "preview-length",
                                           "Content preview length",
                                           "length", "100");
    parser.addOption(previewLengthOption);

    // 解析命令行
    parser.process(app);

    // 获取搜索参数
    QString path = parser.value(pathOption);
    QStringList keywords;

    // 收集所有关键词
    for (int i = 0; i < parser.optionNames().count(); i++) {
        QString optionName = parser.optionNames().at(i);
        if (optionName == "keywords" || optionName == "k") {
            keywords << parser.values(optionName);
        }
    }

    bool caseSensitive = parser.isSet(caseOption);
    bool recursive = parser.isSet(recursiveOption);
    int previewLength = parser.value(previewLengthOption).toInt();

    if (keywords.isEmpty()) {
        qCritical() << "Error: At least one keyword must be specified.";
        parser.showHelp(1);
        return 1;
    }

    QElapsedTimer timer;
    timer.start();

    // 创建文件名搜索引擎
    SearchEngine filenameEngine(SearchType::FileName);
    // 配置文件名搜索选项
    SearchOptions filenameOptions;
    filenameOptions.setSearchPath(path);
    filenameOptions.setCaseSensitive(caseSensitive);
    filenameOptions.setRecursive(recursive);

    // 使用文件名特定API
    FileNameSearchAPI fileNameApi(filenameOptions);
    fileNameApi.setPinyinEnabled(true);
    fileNameApi.setFileTypes(QStringList() << "*.txt" << "*.doc");

    filenameEngine.setSearchOptions(filenameOptions);

    // 创建内容搜索引擎
    SearchEngine contentEngine(SearchType::Content);
    // 配置内容搜索选项
    SearchOptions contentOptions;
    contentOptions.setSearchPath(path);
    contentOptions.setCaseSensitive(caseSensitive);
    contentOptions.setRecursive(recursive);
    contentOptions.setFileTypeFilters(QStringList() << "*.txt" << "*.cpp");
    contentOptions.setMaxPreviewLength(200);

    contentEngine.setSearchOptions(contentOptions);

    // 创建布尔查询（AND逻辑）
    SearchQuery query = SearchQuery::createBooleanQuery(keywords, SearchQuery::BooleanOperator::AND);

    // 记录搜索结果集合
    QSet<QString> filenameResults;
    QSet<QString> contentResults;
    QSet<QString> combinedResults;

    // 连接文件名搜索信号
    QObject::connect(&filenameEngine, &SearchEngine::resultFound,
                     [&filenameResults](const SearchResult &result) {
                         filenameResults.insert(result.path());
                         printFileNameResult(result);
                     });

    // 连接内容搜索信号
    QObject::connect(&contentEngine, &SearchEngine::resultFound,
                     [&contentResults](const SearchResult &result) {
                         contentResults.insert(result.path());
                         printContentResult(result);
                     });

    QObject::connect(&filenameEngine, &SearchEngine::searchFinished,
                     [&filenameResults](const QList<SearchResult> &results) {
                         qInfo() << "文件名搜索完成，共找到" << results.size() << "个结果。";
                     });

    QObject::connect(&contentEngine, &SearchEngine::searchFinished,
                     [&contentResults, &app, &filenameResults, &timer](const QList<SearchResult> &results) {
                         qInfo() << "内容搜索完成，共找到" << results.size() << "个结果。";

                         // 计算交集（同时出现在文件名和内容中的结果）
                         QSet<QString> intersection = filenameResults;
                         intersection.intersect(contentResults);

                         qInfo() << "同时出现在文件名和内容中的结果:" << intersection.size() << "个";
                         if (!intersection.isEmpty()) {
                             qInfo() << "交集文件:";
                             for (const QString &path : intersection) {
                                 qInfo() << "  " << path;
                             }
                         }

                         qInfo() << "总搜索耗时:" << timer.elapsed() << "毫秒";
                         app.quit();
                     });

    QObject::connect(&filenameEngine, &SearchEngine::error, [](const QString &message) {
        qCritical() << "文件名搜索错误:" << message;
    });

    QObject::connect(&contentEngine, &SearchEngine::error, [](const QString &message) {
        qCritical() << "内容搜索错误:" << message;
    });

    // 开始搜索
    qInfo() << "开始搜索关键词:" << keywords.join(", ") << "在路径:" << path;

    // 异步执行两种搜索
    filenameEngine.search(query);
    contentEngine.search(query);

    return app.exec();
}
