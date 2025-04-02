#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
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

using namespace DFM6::Search;

// 实时搜索状态
struct SearchState {
    int fileCount = 0;
    int matchCount = 0;
    qint64 processedSize = 0;
    bool running = false;
    QElapsedTimer timer;
};

// 打印搜索进度
void printProgress(const SearchState& state) {
    qInfo() << "已处理" << state.fileCount << "个文件,"
            << "已找到" << state.matchCount << "个匹配,"
            << "已处理" << (state.processedSize / 1024 / 1024) << "MB,"
            << "已用时" << (state.timer.elapsed() / 1000) << "秒";
}

// 实时打印结果
void printRealtimeResult(SearchState& state, const SearchResult& result, bool isFileName) {
    state.matchCount++;
    
    // 每10个结果更新一次进度
    if (state.matchCount % 10 == 0) {
        printProgress(state);
    }
    
    if (isFileName) {
        qInfo() << "文件名匹配:" << result.path();
        if (result.customAttributes().contains("fileType")) {
            qInfo() << "  类型:" << result.customAttribute("fileType").toString();
        }
    } else {
        qInfo() << "内容匹配:" << result.path();
        if (!result.highlightedContent().isEmpty()) {
            qInfo() << "  内容:" << result.highlightedContent();
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("dfm6-realtime-search");
    app.setApplicationVersion("1.0");
    
    // 命令行解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM6 Realtime Search Example");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 搜索类型选项
    QCommandLineOption searchTypeOption(QStringList() << "t" << "type",
                                     "Search type (filename, content, both)",
                                     "type", "both");
    parser.addOption(searchTypeOption);
    
    // 路径选项
    QCommandLineOption pathOption(QStringList() << "p" << "path",
                                "Search path",
                                "path", QDir::currentPath());
    parser.addOption(pathOption);
    
    // 关键词选项
    QCommandLineOption keywordOption(QStringList() << "k" << "keyword",
                                   "Search keyword",
                                   "keyword");
    parser.addOption(keywordOption);
    
    // 文件类型选项
    QCommandLineOption fileTypeOption(QStringList() << "f" << "file-type",
                                    "File type (e.g., doc, pdf, txt)",
                                    "filetype");
    parser.addOption(fileTypeOption);
    
    // 递归选项
    QCommandLineOption recursiveOption(QStringList() << "r" << "recursive",
                                     "Search recursively");
    parser.addOption(recursiveOption);
    
    // 实时显示选项
    QCommandLineOption realtimeOption(QStringList() << "rt" << "realtime",
                                    "Show results in realtime");
    parser.addOption(realtimeOption);
    
    // 解析命令行
    parser.process(app);
    
    // 获取选项值
    QString searchTypeStr = parser.value(searchTypeOption);
    QString path = parser.value(pathOption);
    QString keyword = parser.value(keywordOption);
    QString fileType = parser.value(fileTypeOption);
    bool recursive = parser.isSet(recursiveOption);
    bool realtime = parser.isSet(realtimeOption);
    
    // 检查参数
    if (keyword.isEmpty() && fileType.isEmpty()) {
        qCritical() << "错误: 必须提供关键词或文件类型！";
        parser.showHelp(1);
        return 1;
    }
    
    // 确保路径存在
    QDir dir(path);
    if (!dir.exists()) {
        qCritical() << "错误: 路径不存在:" << path;
        return 1;
    }
    
    // 创建搜索状态对象
    SearchState fileNameState;
    SearchState contentState;
    fileNameState.timer.start();
    contentState.timer.start();
    fileNameState.running = true;
    contentState.running = true;
    
    // 决定搜索类型
    bool searchFileName = (searchTypeStr == "filename" || searchTypeStr == "both");
    bool searchContent = (searchTypeStr == "content" || searchTypeStr == "both");
    
    // 使用工厂模式创建搜索引擎
    std::shared_ptr<SearchEngine> filenameEngine = SearchFactory::createEngine(SearchType::FileName);
    std::shared_ptr<SearchEngine> contentEngine = SearchFactory::createEngine(SearchType::Content);
    
    // 配置文件名搜索选项
    if (searchFileName) {
        SearchOptions options = filenameEngine->searchOptions();
        options.setSearchPath(path);
        options.setRecursive(recursive);
        options.setSearchMethod(SearchMethod::Realtime);
        
        // 获取文件名搜索特定选项
        FileNameSearchOptions* fileNameOpts = dynamic_cast<FileNameSearchOptions*>(&options);
        if (fileNameOpts) {
            // 配置文件类型
            if (!fileType.isEmpty()) {
                QStringList types;
                for (const QString& type : fileType.split(",")) {
                    types << "*." + type.trimmed();
                }
                fileNameOpts->setFileTypes(types);
                qInfo() << "搜索文件类型:" << types.join(", ");
            }
            
            // 启用拼音搜索和模糊搜索
            fileNameOpts->setPinyinEnabled(true);
            fileNameOpts->setFuzzySearch(true);
        }
        
        // 使用文件名特定API
        FileNameSearchAPI fileNameApi(options);
        fileNameApi.setPinyinEnabled(true);
        fileNameApi.setFileTypes(types);
        
        filenameEngine->setSearchOptions(options);
        
        // 添加实时进度显示
        QTimer progressTimer;
        QObject::connect(&progressTimer, &QTimer::timeout, [&fileNameState]() {
            printProgress(fileNameState);
        });
        progressTimer.start(1000); // 每秒更新一次进度
        
        // 使用回调实时处理结果
        filenameEngine->searchWithCallback(query, [&fileNameState](const SearchResult& result) {
            // 立即输出结果，真正实现实时显示
            qInfo() << "找到:" << result.path();
            fileNameState.matchCount++;
            fileNameState.processedSize += result.size();
            return true; // 继续搜索
        });
    }
    
    // 配置内容搜索选项
    if (searchContent) {
        SearchOptions options = contentEngine->searchOptions();
        options.setSearchPath(path);
        options.setRecursive(recursive);
        
        // 获取内容搜索特定选项
        ContentSearchOptions* contentOpts = dynamic_cast<ContentSearchOptions*>(&options);
        if (contentOpts) {
            // 设置文件类型过滤器
            if (!fileType.isEmpty()) {
                QStringList types;
                for (const QString& type : fileType.split(",")) {
                    types << "*." + type.trimmed();
                }
                contentOpts->setFileTypeFilters(types);
            } else {
                // 默认搜索常见文本文件类型
                contentOpts->setFileTypeFilters(QStringList() 
                    << "*.txt" << "*.md" << "*.cpp" << "*.h" 
                    << "*.py" << "*.java" << "*.html" << "*.xml" << "*.json");
            }
            
            // 不搜索二进制文件
            contentOpts->setSearchBinaryFiles(false);
            
            // 设置较大的预览长度
            contentOpts->setMaxPreviewLength(200);
        }
        
        contentEngine->setSearchOptions(options);
    }
    
    // 创建查询
    SearchQuery query;
    if (!keyword.isEmpty()) {
        query = SearchQuery::createSimpleQuery(keyword);
    } else {
        // 如果只提供文件类型，创建一个匹配所有内容的查询
        query = SearchQuery::createSimpleQuery("*");
    }
    
    // 连接信号和槽
    QObject::connect(filenameEngine.get(), &SearchEngine::progressChanged,
                    [&fileNameState](int current, int total) {
        fileNameState.fileCount = current;
        if (current % 100 == 0) {
            printProgress(fileNameState);
        }
    });
    
    QObject::connect(contentEngine.get(), &SearchEngine::progressChanged,
                    [&contentState](int current, int total) {
        contentState.fileCount = current;
        if (current % 100 == 0) {
            printProgress(contentState);
        }
    });
    
    // 处理错误
    QObject::connect(filenameEngine.get(), &SearchEngine::error,
                    [](const QString& msg) {
        qCritical() << "文件名搜索错误:" << msg;
    });
    
    QObject::connect(contentEngine.get(), &SearchEngine::error,
                    [](const QString& msg) {
        qCritical() << "内容搜索错误:" << msg;
    });
    
    // 搜索完成处理
    QObject::connect(filenameEngine.get(), &SearchEngine::searchFinished,
                    [&fileNameState, searchContent, &app](const QList<SearchResult>& results) {
        fileNameState.running = false;
        qInfo() << "文件名搜索完成，耗时" << fileNameState.timer.elapsed() / 1000 << "秒，找到" 
                << results.size() << "个结果";
        
        // 如果不搜索内容且非实时模式，则显示所有结果
        if (!searchContent) {
            if (results.isEmpty()) {
                qInfo() << "未找到匹配的文件";
            } else {
                qInfo() << "找到以下文件:";
                for (const SearchResult& result : results) {
                    qInfo() << "  " << result.path();
                }
            }
            app.quit();
        }
    });
    
    QObject::connect(contentEngine.get(), &SearchEngine::searchFinished,
                    [&contentState, &fileNameState, &app](const QList<SearchResult>& results) {
        contentState.running = false;
        qInfo() << "内容搜索完成，耗时" << contentState.timer.elapsed() / 1000 << "秒，找到" 
                << results.size() << "个结果";
        
        // 如果文件名搜索也完成，则退出应用
        if (!fileNameState.running) {
            app.quit();
        }
    });
    
    // 处理特定类型的结果
    QObject::connect(filenameEngine.get(), &SearchEngine::resultFound,
                    [&fileNameState](const SearchResultBase &result) {
        if (result.resultType() == SearchType::FileName) {
            const FileNameSearchResult* fileResult = 
                static_cast<const FileNameSearchResult*>(&result);
            
            fileNameState.matchCount++;
            qInfo() << "文件名匹配:" << fileResult->path();
            qInfo() << "  匹配分数:" << fileResult->matchScore();
        }
    });
    
    // 根据实时显示选项决定使用哪种搜索方式
    if (realtime) {
        // 使用回调实时显示结果
        if (searchFileName) {
            qInfo() << "开始实时文件名搜索...";
            filenameEngine->searchWithCallback(query, [&fileNameState](const SearchResult& result) {
                printRealtimeResult(fileNameState, result, true);
                fileNameState.processedSize += result.size();
                return true; // 继续搜索
            });
        }
        
        if (searchContent) {
            qInfo() << "开始实时内容搜索...";
            contentEngine->searchWithCallback(query, [&contentState](const SearchResult& result) {
                printRealtimeResult(contentState, result, false);
                contentState.processedSize += result.size();
                return true; // 继续搜索
            });
        }
    } else {
        // 使用一次性返回结果的方式
        if (searchFileName) {
            qInfo() << "开始文件名搜索...";
            filenameEngine->search(query);
        }
        
        if (searchContent) {
            qInfo() << "开始内容搜索...";
            contentEngine->search(query);
        }
    }
    
    return app.exec();
} 
