#include <QApplication>
#include <QCommandLineParser>
#include "advanced_search_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QSearch Advanced Example");
    app.setApplicationVersion("1.0.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("高级搜索示例应用");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加命令行选项
    QCommandLineOption useWorkerOption(
        QStringList() << "w" << "use-worker",
        "使用独立的工作进程 (如果可用)");
    parser.addOption(useWorkerOption);
    
    QCommandLineOption indexPathOption(
        QStringList() << "i" << "index-path",
        "指定索引路径", "path", QDir::homePath() + "/.qsearch");
    parser.addOption(indexPathOption);
    
    parser.process(app);
    
    // 创建主窗口
    AdvancedSearchWindow window;
    
    // 设置选项
    window.setUseWorker(parser.isSet(useWorkerOption));
    window.setIndexPath(parser.value(indexPathOption));
    
    window.show();
    
    return app.exec();
} 