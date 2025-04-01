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
    parser.addPositionalArgument("keyword", "Keyword to search for in file contents");

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

    // Process the arguments
    parser.process(app);

    // Get keyword
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "Error: No keyword specified.";
        parser.showHelp(1);
        return 1;
    }

    const QString keyword = args.first();

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

    try {
        // Create searcher
        ContentSearcher searcher(indexPath);

        // Perform search
        qInfo() << "Searching for:" << keyword;
        QList<SearchResult> results = searcher.search(keyword, maxResults);

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
