#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include "contentsearcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("file-searcher");
    QCoreApplication::setApplicationVersion("1.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Search for files based on content");
    parser.addHelpOption();
    parser.addVersionOption();

    // Add keyword argument
    parser.addPositionalArgument("keyword", "Keyword(s) to search for in file contents");

    // Add options
    QCommandLineOption indexPathOption(
            QStringList() << "i"
                          << "index-path",
            "Path to the index directory (default: ~/.config/deepin/dde-file-manager/index)",
            "path");
    parser.addOption(indexPathOption);

    QCommandLineOption maxResultsOption(
            QStringList() << "m"
                          << "max-results",
            "Maximum number of results to display (default: 100000)",
            "number",
            "100000");
    parser.addOption(maxResultsOption);
    
    // 添加布尔"与"搜索选项
    QCommandLineOption booleanAndOption(
            QStringList() << "a"
                          << "and",
            "Perform boolean AND search (all keywords must be present)");
    parser.addOption(booleanAndOption);
    
    // 添加搜索路径选项
    QCommandLineOption searchPathOption(
            QStringList() << "p"
                          << "path",
            "Path to search within (default: user's home directory)",
            "path",
            QDir::homePath());
    parser.addOption(searchPathOption);

    // Process the arguments
    parser.process(app);

    // Get keywords
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "Error: No keyword specified.";
        parser.showHelp(1);
        return 1;
    }

    // 获取搜索关键词
    QStringList keywords;
    if (parser.isSet(booleanAndOption)) {
        // 布尔搜索模式：多个关键词
        keywords = args;
    } else {
        // 普通搜索模式：只取第一个关键词
        keywords << args.first();
    }

    // Get index path
    QString indexPath;
    if (parser.isSet(indexPathOption)) {
        indexPath = parser.value(indexPathOption);
    } else {
        indexPath = QDir::homePath() + "/.config/deepin/dde-file-manager/index";
    }

    // Verify index path exists
    if (!QDir(indexPath).exists()) {
        qCritical() << "Error: Index directory does not exist:" << indexPath;
        return 1;
    }

    // Get max results
    bool ok;
    int maxResults = parser.value(maxResultsOption).toInt(&ok);
    if (!ok || maxResults <= 0) {
        qCritical() << "Error: Invalid value for max-results option.";
        return 1;
    }
    
    // 获取搜索路径
    QString searchPath = parser.value(searchPathOption);

    try {
        // Create searcher
        ContentSearcher searcher(indexPath);

        // Perform search
        QList<SearchResult> results;
        
        if (parser.isSet(booleanAndOption)) {
            // 布尔"与"搜索
            qInfo() << "Performing boolean AND search for:" << keywords.join(", ");
            results = searcher.booleanAndSearch(keywords, maxResults, searchPath);
        } else {
            // 普通搜索
            qInfo() << "Searching for:" << keywords.first();
            results = searcher.search(keywords.first(), maxResults, searchPath);
        }

        // Display results
        if (results.isEmpty()) {
            qInfo() << "No results found.";
        } else {
            qInfo() << "Found" << results.size() << "results:";
            qInfo() << "--------------------------";

            for (int i = 0; i < results.size(); ++i) {
                const SearchResult &result = results.at(i);

                qInfo() << "Result" << (i + 1) << ":";
                qInfo() << "Path:" << result.path;

                // Convert timestamp to readable date
                QDateTime dt = QDateTime::fromSecsSinceEpoch(result.modifiedTime.toLongLong());
                qInfo() << "Modified:" << dt.toString(Qt::ISODate);

                qInfo() << "Content:";
                qInfo().noquote() << result.highlightedContent;
                qInfo() << "--------------------------";
            }
        }
    } catch (const std::exception &e) {
        qCritical() << "Error:" << e.what();
        return 1;
    }

    return 0;
}
