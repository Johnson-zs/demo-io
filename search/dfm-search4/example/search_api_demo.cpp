#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDir>
#include <QThread>
#include <QTimer>

#include <dfm6-search/searchengine.h>
#include <dfm6-search/searchquery.h>
#include <dfm6-search/searchfactory.h>
#include <dfm6-search/searchresult.h>
#include <dfm6-search/filenamesearchapi.h>
#include <dfm6-search/contentsearchapi.h>

using namespace DFM6::Search;

/**
 * 这个示例展示了搜索API的全面用法，包括：
 * 1. 创建和配置不同类型的搜索引擎
 * 2. 使用各种查询类型
 * 3. 处理不同类型的搜索结果
 * 4. 搜索回调和事件处理
 * 5. 各种搜索选项设置方法
 */

// 通用结果处理函数
void handleSearchResult(const SearchResult *result)
{
    if (!result) {
        qWarning() << "收到了空搜索结果";
        return;
    }

    if (result->resultType() == SearchType::FileName) {
        const FileNameSearchResult *fileResult =
                static_cast<const FileNameSearchResult *>(result);

        qInfo() << "文件名结果:" << fileResult->path();
        qInfo() << "  - 匹配类型:" << fileResult->matchType();
        qInfo() << "  - 大小:" << fileResult->size() << "字节";
        qInfo() << "  - 修改时间:" << fileResult->modifiedTime().toString();
    } else if (result->resultType() == SearchType::Content) {
        const ContentSearchResult *contentResult =
                static_cast<const ContentSearchResult *>(result);

        qInfo() << "内容结果:" << contentResult->path();
        qInfo() << "  - 内容片段:" << contentResult->highlightedContent().trimmed();
        if (contentResult->lineNumber() >= 0) {
            qInfo() << "  - 行号:" << contentResult->lineNumber();
        }
    }
}

// 演示使用工厂创建并配置搜索引擎
std::shared_ptr<SearchEngine> setupSearchEngine(SearchType type, const QString &path, bool caseSensitive)
{
    // 使用工厂创建引擎
    auto engine = SearchFactory::createEngine(type);
    if (!engine) {
        qCritical() << "创建引擎失败";
        return nullptr;
    }

    // 获取选项并设置基本参数
    SearchOptions options = engine->searchOptions();
    options.setSearchPath(path);
    options.setCaseSensitive(caseSensitive);
    options.setRecursive(true);
    options.setMaxResults(100);

    // 根据搜索类型配置特定选项
    if (type == SearchType::FileName) {
        // 使用文件名搜索API设置特定选项
        FileNameSearchAPI fileNameApi(options);
        fileNameApi.setPinyinEnabled(true);
        fileNameApi.setFuzzySearch(true);
        fileNameApi.setFileTypes(QStringList() << "*.txt"
                                               << "*.cpp"
                                               << "*.h");
    } else if (type == SearchType::Content) {
        // 使用内容搜索API设置特定选项
        ContentSearchAPI contentApi(options);
        contentApi.setFileTypeFilters(QStringList() << "*.txt"
                                                    << "*.cpp"
                                                    << "*.h");
        contentApi.setMaxPreviewLength(150);
        contentApi.setSearchBinaryFiles(false);
    }

    // 设置选项到引擎
    engine->setSearchOptions(options);
    return engine;
}

// 创建不同类型的查询
SearchQuery createQuery(const QString &keyword, bool isWildcard, bool isRegex, bool isFuzzy)
{
    if (isWildcard) {
        return SearchQuery::createWildcardQuery(keyword);
    } else if (isRegex) {
        return SearchQuery::createRegexQuery(keyword);
    } else if (isFuzzy) {
        return SearchQuery::createFuzzyQuery(keyword);
    } else {
        return SearchQuery::createSimpleQuery(keyword);
    }
}

// 演示同步搜索
void demoSyncSearch(SearchEngine *engine, const SearchQuery &query)
{
    qInfo() << "开始同步搜索...";
    QElapsedTimer timer;
    timer.start();

    QList<SearchResult> results = engine->searchSync(query);

    qInfo() << "搜索完成，耗时" << timer.elapsed() << "毫秒，找到" << results.size() << "个结果";

    // 处理结果
    for (const auto &result : results) {
        handleSearchResult(&result);
    }
}

// 演示异步搜索
void demoAsyncSearch(SearchEngine *engine, const SearchQuery &query, QCoreApplication &app)
{
    qInfo() << "开始异步搜索...";
    QElapsedTimer timer;
    timer.start();

    // 连接信号
    QObject::connect(engine, &SearchEngine::resultFound,
                     [](const SearchResult &result) {
                         handleSearchResult(&result);
                     });

    QObject::connect(engine, &SearchEngine::searchFinished,
                     [&timer, &app](const QList<SearchResult> &results) {
                         qInfo() << "搜索完成，耗时" << timer.elapsed() << "毫秒，找到" << results.size() << "个结果";
                         app.quit();
                     });

    QObject::connect(engine, &SearchEngine::progressChanged,
                     [](int current, int total) {
                         qInfo() << "进度:" << current << "/" << total;
                     });

    // 开始搜索
    engine->search(query);
}

// 演示使用回调的实时搜索
void demoCallbackSearch(SearchEngine *engine, const SearchQuery &query)
{
    qInfo() << "开始回调搜索...";
    QElapsedTimer timer;
    timer.start();

    int count = 0;
    engine->searchWithCallback(query, [&count](const SearchResult &result) {
        handleSearchResult(&result);
        count++;
        // 假设我们想在找到10个结果后停止
        return count < 10;
    });

    qInfo() << "搜索完成，耗时" << timer.elapsed() << "毫秒，处理了" << count << "个结果";
}

// 演示组合搜索
void demoCombinedSearch(const QString &keyword, const QString &path, QCoreApplication &app)
{
    qInfo() << "演示组合搜索 - 同时搜索文件名和内容...";

    // 设置文件名搜索
    auto filenameEngine = setupSearchEngine(SearchType::FileName, path, false);

    // 设置内容搜索
    auto contentEngine = setupSearchEngine(SearchType::Content, path, false);

    // 创建查询
    auto query = SearchQuery::createSimpleQuery(keyword);

    // 存储结果集
    QSet<QString> filenameResults;
    QSet<QString> contentResults;

    // 连接文件名搜索信号
    QObject::connect(filenameEngine.get(), &SearchEngine::resultFound,
                     [&filenameResults](const SearchResult &result) {
                         filenameResults.insert(result.path());
                         qInfo() << "文件名匹配:" << result.path();
                     });

    // 连接内容搜索信号
    QObject::connect(contentEngine.get(), &SearchEngine::resultFound,
                     [&contentResults](const SearchResult &result) {
                         contentResults.insert(result.path());
                         qInfo() << "内容匹配:" << result.path();
                     });

    // 处理完成事件
    QObject::connect(contentEngine.get(), &SearchEngine::searchFinished,
                     [&filenameResults, &contentResults, &app](const QList<SearchResult> &) {
                         // 计算交集（同时出现在文件名和内容中的结果）
                         QSet<QString> intersection = filenameResults;
                         intersection.intersect(contentResults);

                         qInfo() << "文件名搜索结果:" << filenameResults.size() << "个";
                         qInfo() << "内容搜索结果:" << contentResults.size() << "个";
                         qInfo() << "同时匹配的结果:" << intersection.size() << "个";

                         if (!intersection.isEmpty()) {
                             qInfo() << "同时匹配的文件:";
                             for (const QString &path : intersection) {
                                 qInfo() << "  " << path;
                             }
                         }

                         app.quit();
                     });

    // 执行搜索
    filenameEngine->search(query);
    contentEngine->search(query);
}

// 演示搜索工厂的使用
void demoSearchFactory()
{
    qInfo() << "演示工厂功能...";

    // 创建各种类型的搜索结果
    auto fileResult = SearchFactory::instance().createResult(SearchType::FileName, "/path/to/file.txt");
    auto contentResult = SearchFactory::instance().createResult(SearchType::Content, "/path/to/content.txt");

    // 创建各种类型的搜索选项
    auto fileOptions = SearchFactory::instance().createOptions(SearchType::FileName);
    auto contentOptions = SearchFactory::instance().createOptions(SearchType::Content);

    qInfo() << "成功创建选项和结果对象";
}

// 演示索引和非索引搜索模式
void demoSearchMethods(const QString &path, const QString &keyword)
{
    qInfo() << "演示索引和非索引搜索模式...";

    // 1. 非索引搜索
    qInfo() << "===== 非索引搜索 =====";
    auto nonIndexedEngine = SearchFactory::createEngine(SearchType::FileName);
    SearchOptions nonIdxOptions = nonIndexedEngine->searchOptions();
    nonIdxOptions.setSearchPath(path);
    nonIdxOptions.setSearchMethod(SearchMethod::Realtime);
    nonIndexedEngine->setSearchOptions(nonIdxOptions);

    QElapsedTimer timer;
    timer.start();

    QList<SearchResult> nonIdxResults = nonIndexedEngine->searchSync(
            SearchQuery::createSimpleQuery(keyword));

    qInfo() << "非索引搜索耗时:" << timer.elapsed() << "毫秒, 找到"
            << nonIdxResults.size() << "个结果";

    // 2. 索引搜索
    qInfo() << "===== 索引搜索 =====";
    auto indexedEngine = SearchFactory::createEngine(SearchType::FileName);
    SearchOptions idxOptions = indexedEngine->searchOptions();
    idxOptions.setSearchPath(path);
    idxOptions.setSearchMethod(SearchMethod::Indexed);
    // 设置索引路径（实际应用中可能是配置的路径）
    idxOptions.setCustomOption("indexPath", QDir::tempPath() + "/search_index");
    indexedEngine->setSearchOptions(idxOptions);

    // 索引构建过程监控
    QObject::connect(indexedEngine.get(), &SearchEngine::progressChanged,
                     [](int current, int total) {
                         qInfo() << "索引构建进度:" << current << "/" << total;
                     });

    QObject::connect(indexedEngine.get(), &SearchEngine::statusChanged,
                     [](SearchStatus status) {
                         if (status == SearchStatus::BuildingIndex) {
                             qInfo() << "正在构建索引...";
                         } else if (status == SearchStatus::Ready) {
                             qInfo() << "索引已就绪，准备搜索";
                         }
                     });

    // 等待索引就绪
    while (indexedEngine->status() != SearchStatus::Ready && indexedEngine->status() != SearchStatus::Error) {
        QThread::msleep(100);
        QCoreApplication::processEvents();
    }

    // 执行索引搜索
    timer.restart();
    QList<SearchResult> idxResults = indexedEngine->searchSync(
            SearchQuery::createSimpleQuery(keyword));

    qInfo() << "索引搜索耗时:" << timer.elapsed() << "毫秒, 找到"
            << idxResults.size() << "个结果";

    // 比较两种方式
    qInfo() << "索引搜索与非索引搜索性能对比:";
    if (nonIdxResults.size() == idxResults.size()) {
        qInfo() << "结果数量一致，索引机制工作正常";
    } else {
        qWarning() << "结果数量不一致，可能存在索引问题";
    }
}

// 演示搜索流程控制
void demoSearchControl(QCoreApplication &app, const QString &path)
{
    qInfo() << "演示搜索流程控制...";

    // 创建一个内容搜索引擎（通常内容搜索耗时更长，便于演示控制）
    auto engine = SearchFactory::createEngine(SearchType::Content);
    SearchOptions options = engine->searchOptions();
    options.setSearchPath(path);
    // 使用广泛的查询，确保有足够多的结果
    options.setRecursive(true);
    engine->setSearchOptions(options);

    // 监控搜索状态
    QObject::connect(engine.get(), &SearchEngine::statusChanged,
                     [](SearchStatus status) {
                         switch (status) {
                         case SearchStatus::Ready:
                             qInfo() << "搜索引擎就绪";
                             break;
                         case SearchStatus::Searching:
                             qInfo() << "正在搜索...";
                             break;
                         case SearchStatus::Paused:
                             qInfo() << "搜索已暂停";
                             break;
                         case SearchStatus::Cancelled:
                             qInfo() << "搜索已取消";
                             break;
                         case SearchStatus::Error:
                             qInfo() << "搜索发生错误";
                             break;
                         case SearchStatus::BuildingIndex:
                             qInfo() << "正在构建索引";
                             break;
                         default:
                             qInfo() << "未知状态:" << static_cast<int>(status);
                         }
                     });

    // 监控进度
    QObject::connect(engine.get(), &SearchEngine::progressChanged,
                     [](int current, int total) {
                         qInfo() << "进度:" << current << "/" << total;
                     });

    // 记录结果数
    int resultCount = 0;
    QObject::connect(engine.get(), &SearchEngine::resultFound,
                     [&resultCount](const SearchResult &) {
                         resultCount++;
                         if (resultCount % 10 == 0) {
                             qInfo() << "已找到" << resultCount << "个结果";
                         }
                     });

    // 监控搜索完成
    QObject::connect(engine.get(), &SearchEngine::searchFinished,
                     [&app](const QList<SearchResult> &results) {
                         qInfo() << "搜索结束，总共找到" << results.size() << "个结果";
                         app.quit();
                     });

    // 创建查询 - 使用通配符确保匹配多个文件
    SearchQuery query = SearchQuery::createWildcardQuery("*");

    // 开始搜索
    engine->search(query);

    // 设置定时器在搜索开始后执行控制操作
    QTimer::singleShot(1000, [engine]() {
        if (engine->status() == SearchStatus::Searching) {
            qInfo() << "暂停搜索...";
            engine->pause();

            // 2秒后恢复
            QTimer::singleShot(2000, [engine]() {
                qInfo() << "恢复搜索...";
                engine->resume();

                // 再过3秒后取消搜索
                QTimer::singleShot(3000, [engine]() {
                    qInfo() << "取消搜索...";
                    engine->cancel();
                });
            });
        }
    });
}

// 演示限制搜索结果和优先级搜索
void demoSearchLimits(const QString &path)
{
    qInfo() << "演示搜索限制和优先级...";

    // 创建引擎
    auto engine = SearchFactory::createEngine(SearchType::FileName);

    // 配置有限制的选项
    SearchOptions options = engine->searchOptions();
    options.setSearchPath(path);
    options.setRecursive(true);
    // 限制最大结果数
    options.setMaxResults(20);
    // 设置优先搜索的文件类型
    FileNameSearchAPI fileApi(options);
    fileApi.setFileTypes(QStringList() << "*.txt"
                                       << "*.cpp"
                                       << "*.h");

    engine->setSearchOptions(options);

    // 执行搜索
    QElapsedTimer timer;
    timer.start();

    int resultCount = 0;
    engine->searchWithCallback(SearchQuery::createWildcardQuery("*"),
                               [&resultCount](const SearchResult &result) {
                                   resultCount++;
                                   qInfo() << resultCount << ". " << result.path();
                                   return true;   // 继续搜索
                               });

    qInfo() << "限制搜索耗时:" << timer.elapsed() << "毫秒, 返回前"
            << resultCount << "个结果";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 命令行解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM6 Search API Demo");
    parser.addHelpOption();

    // 搜索类型选项
    QCommandLineOption typeOption("type", "搜索类型 (filename, content, combined)", "type", "combined");
    parser.addOption(typeOption);

    // 搜索模式选项
    QCommandLineOption modeOption("mode", "搜索模式 (sync, async, callback)", "mode", "async");
    parser.addOption(modeOption);

    // 关键词选项
    QCommandLineOption keywordOption("keyword", "搜索关键词", "keyword", "test");
    parser.addOption(keywordOption);

    // 路径选项
    QCommandLineOption pathOption("path", "搜索路径", "path", QDir::currentPath());
    parser.addOption(pathOption);

    // 查询类型选项
    QCommandLineOption wildcardOption("wildcard", "使用通配符搜索");
    parser.addOption(wildcardOption);

    QCommandLineOption regexOption("regex", "使用正则表达式搜索");
    parser.addOption(regexOption);

    QCommandLineOption fuzzyOption("fuzzy", "使用模糊搜索");
    parser.addOption(fuzzyOption);

    // 添加新的命令行选项
    QCommandLineOption methodOption("method", "演示搜索方法 (index/nonindex)");
    parser.addOption(methodOption);

    QCommandLineOption controlOption("control", "演示搜索流程控制");
    parser.addOption(controlOption);

    QCommandLineOption limitsOption("limits", "演示搜索限制");
    parser.addOption(limitsOption);

    parser.process(app);

    // 获取选项
    QString typeStr = parser.value(typeOption);
    QString modeStr = parser.value(modeOption);
    QString keyword = parser.value(keywordOption);
    QString path = parser.value(pathOption);
    bool isWildcard = parser.isSet(wildcardOption);
    bool isRegex = parser.isSet(regexOption);
    bool isFuzzy = parser.isSet(fuzzyOption);

    // 演示工厂功能
    demoSearchFactory();

    // 处理新增的演示选项
    if (parser.isSet(methodOption)) {
        demoSearchMethods(path, keyword);
        return 0;
    }

    if (parser.isSet(controlOption)) {
        demoSearchControl(app, path);
        return app.exec();
    }

    if (parser.isSet(limitsOption)) {
        demoSearchLimits(path);
        return 0;
    }

    // 处理组合搜索
    if (typeStr == "combined") {
        demoCombinedSearch(keyword, path, app);
        return app.exec();
    }

    // 确定搜索类型
    SearchType searchType = SearchType::FileName;
    if (typeStr == "content") {
        searchType = SearchType::Content;
    }

    // 创建并配置搜索引擎
    auto engine = setupSearchEngine(searchType, path, false);
    if (!engine) {
        return 1;
    }

    // 创建查询
    SearchQuery query = createQuery(keyword, isWildcard, isRegex, isFuzzy);

    // 根据模式执行不同类型的搜索
    if (modeStr == "sync") {
        demoSyncSearch(engine.get(), query);
        return 0;
    } else if (modeStr == "callback") {
        demoCallbackSearch(engine.get(), query);
        return 0;
    } else {
        // 默认使用异步搜索
        demoAsyncSearch(engine.get(), query, app);
        return app.exec();
    }
}
