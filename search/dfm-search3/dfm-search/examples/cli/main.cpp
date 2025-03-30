#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDir>
#include <iostream>

#include "dfm-search/searchmanager.h"
#include "dfm-search/filenamedataprovider.h"
#include "dfm-search/filecontentdataprovider.h"

using namespace DFMSearch;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dfm-search-cli");
    QCoreApplication::setApplicationVersion("0.1.0");
    
    QTextStream out(stdout);
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("命令行搜索示例应用");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加搜索关键词参数
    parser.addPositionalArgument("keyword", "搜索关键词");
    
    // 添加搜索路径参数
    QCommandLineOption pathOption(QStringList() << "p" << "path", 
                                  "搜索路径（默认为当前目录）", 
                                  "path", 
                                  QDir::currentPath());
    parser.addOption(pathOption);
    
    // 添加搜索类型参数
    QCommandLineOption typeOption(QStringList() << "t" << "type", 
                                  "搜索类型（file=文件名，content=文件内容，all=全部）", 
                                  "type", 
                                  "all");
    parser.addOption(typeOption);
    
    // 添加大小写敏感参数
    QCommandLineOption caseSensitiveOption(QStringList() << "c" << "case-sensitive", 
                                           "启用大小写敏感搜索");
    parser.addOption(caseSensitiveOption);
    
    // 添加正则表达式参数
    QCommandLineOption regexOption(QStringList() << "r" << "regex", 
                                   "使用正则表达式搜索");
    parser.addOption(regexOption);
    
    // 添加模糊匹配参数
    QCommandLineOption fuzzyOption(QStringList() << "f" << "fuzzy", 
                                   "使用模糊匹配搜索");
    parser.addOption(fuzzyOption);
    
    // 添加结果限制参数
    QCommandLineOption limitOption(QStringList() << "l" << "limit", 
                                   "限制结果数量", 
                                   "limit", 
                                   "100");
    parser.addOption(limitOption);
    
    // 解析参数
    parser.process(app);
    
    // 获取关键词
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        out << "错误: 必须提供搜索关键词\n";
        parser.showHelp(1);
        return 1;
    }
    const QString keyword = args.first();
    
    // 获取搜索路径
    const QString searchPath = parser.value(pathOption);
    
    // 获取搜索类型
    const QString typeStr = parser.value(typeOption).toLower();
    bool searchFiles = typeStr == "file" || typeStr == "all";
    bool searchContent = typeStr == "content" || typeStr == "all";
    
    if (!searchFiles && !searchContent) {
        out << "错误: 无效的搜索类型: " << typeStr << "\n";
        return 1;
    }
    
    // 获取其他选项
    const bool caseSensitive = parser.isSet(caseSensitiveOption);
    const bool useRegex = parser.isSet(regexOption);
    const bool useFuzzy = parser.isSet(fuzzyOption);
    const int limit = parser.value(limitOption).toInt();
    
    // 创建搜索查询
    SearchQuery query(keyword);
    query.addSearchPath(searchPath);
    query.setLimit(limit);
    
    // 设置搜索标志
    if (caseSensitive) {
        query.addFlag(SearchFlag::CaseSensitive);
    }
    if (useRegex) {
        query.addFlag(SearchFlag::RegexSupport);
    }
    if (useFuzzy) {
        query.addFlag(SearchFlag::FuzzyMatch);
    }
    
    // 获取搜索管理器实例
    SearchManager* manager = SearchManager::instance();
    
    // 注册提供者
    if (searchFiles) {
        manager->registerProvider([]() -> std::shared_ptr<SearchProvider> {
            return std::make_shared<FileNameDataProvider>();
        }, SearchType::FileName);
    }
    
    if (searchContent) {
        manager->registerProvider([]() -> std::shared_ptr<SearchProvider> {
            return std::make_shared<FileContentDataProvider>();
        }, SearchType::FileContent);
    }
    
    // 设置搜索类型
    std::vector<SearchType> types;
    if (searchFiles) {
        types.push_back(SearchType::FileName);
    }
    if (searchContent) {
        types.push_back(SearchType::FileContent);
    }
    manager->setSearchTypes(types);
    
    // 设置搜索查询
    manager->setQuery(query);
    
    // 设置信号处理
    QObject::connect(manager, &SearchManager::resultFound, 
                     [&out](const SearchResult& result) {
        out << result.toString() << "\n";
    });
    
    QObject::connect(manager, &SearchManager::searchCompleted, 
                     []() {
        std::cout << "\n搜索完成！" << std::endl;
        QCoreApplication::quit();
    });
    
    QObject::connect(manager, &SearchManager::searchError, 
                     [&out](const QString& error) {
        out << "搜索错误: " << error << "\n";
        QCoreApplication::quit();
    });
    
    QObject::connect(manager, &SearchManager::progressChanged, 
                     [](int progress) {
        // 显示进度
        std::cout << "\r搜索进度: " << progress << "%" << std::flush;
    });
    
    // 开始搜索
    out << "开始搜索: " << keyword << "\n";
    out << "搜索路径: " << searchPath << "\n";
    out << "搜索类型: " << (searchFiles ? "文件名 " : "") 
                        << (searchContent ? "文件内容" : "") << "\n";
    out << "选项: " 
        << (caseSensitive ? "大小写敏感 " : "") 
        << (useRegex ? "正则表达式 " : "") 
        << (useFuzzy ? "模糊匹配 " : "") << "\n";
    out << "结果限制: " << limit << "\n\n";
    
    if (!manager->start()) {
        out << "无法启动搜索\n";
        return 1;
    }
    
    return app.exec();
} 