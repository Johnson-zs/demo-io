#include <QCoreApplication>
#include <QDebug>
#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchfactory.h>
#include <dfm6-search/desktopsearchapi.h>
#include <QTimer>

using namespace DFM6::Search;

void printResult(const SearchResult &result)
{
    qInfo() << "========= Application Found =========";
    qInfo() << "Name:" << result.customAttribute("applicationName").toString();
    qInfo() << "Path:" << result.path();
    qInfo() << "Description:" << result.customAttribute("description").toString();
    qInfo() << "Icon:" << result.customAttribute("icon").toString();
    qInfo() << "Exec:" << result.customAttribute("exec").toString();
    qInfo() << "Categories:" << result.customAttribute("categories").toString();
    qInfo() << "Score:" << result.score();
    qInfo() << "===================================";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qInfo() << "Desktop Search Demo";
    qInfo() << "-------------------";
    
    // 获取查询关键词
    QString keyword;
    if (argc > 1) {
        keyword = argv[1];
    } else {
        qInfo() << "Usage: desktop_search_demo <keyword>";
        qInfo() << "Using default keyword: 'terminal'";
        keyword = "terminal";
    }
    
    // 创建桌面应用搜索引擎
    SearchEngine *engine = SearchFactory::createEngine(SearchType::Desktop);
    
    // 配置搜索选项
    DesktopSearchAPI desktopOptions = engine->desktopOptions();
    desktopOptions.setSearchName(true);
    desktopOptions.setSearchDescription(true);
    desktopOptions.setSearchKeywords(true);
    desktopOptions.setOnlyShowVisible(true);
    
    // 限制搜索类别（可选）
    // desktopOptions.setCategories(QStringList() << "System" << "Utility");
    
    // 连接信号
    QObject::connect(engine, &SearchEngine::searchStarted, []() {
        qInfo() << "Search started...";
    });
    
    QObject::connect(engine, &SearchEngine::resultFound, [](const SearchResult &result) {
        printResult(result);
    });
    
    QObject::connect(engine, &SearchEngine::searchFinished, [](const QList<SearchResult> &results) {
        qInfo() << "Search finished. Found" << results.size() << "applications.";
        QCoreApplication::quit();
    });
    
    QObject::connect(engine, &SearchEngine::error, [](const QString &message) {
        qWarning() << "Search error:" << message;
    });
    
    // 创建查询并执行
    SearchQuery query = SearchFactory::createQuery(keyword);
    
    // 使用回调方式搜索，以便实时获取结果
    engine->searchWithCallback(query, printResult);
    
    return app.exec();
} 