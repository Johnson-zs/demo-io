#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "mainwindow.h"
#include <dfm-search/dfm_search.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DFM-Search Example");
    app.setApplicationVersion(DFM::Search::version());
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM搜索库示例应用程序");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加搜索类型选项
    QCommandLineOption searchTypeOption(
        QStringList() << "t" << "type",
        "指定搜索类型 (filename, content, app)",
        "type",
        "filename"
    );
    parser.addOption(searchTypeOption);
    
    // 添加搜索路径选项
    QCommandLineOption searchPathOption(
        QStringList() << "p" << "path",
        "指定搜索路径",
        "path",
        QDir::homePath()
    );
    parser.addOption(searchPathOption);
    
    // 添加索引选项
    QCommandLineOption indexOption(
        QStringList() << "i" << "indexed",
        "使用索引搜索"
    );
    parser.addOption(indexOption);
    
    // 处理命令行参数
    parser.process(app);
    
    // 初始化搜索库
    if (!DFM::Search::initialize()) {
        qCritical() << "初始化搜索库失败!";
        return 1;
    }
    
    // 获取命令行参数
    QString searchType = parser.value(searchTypeOption);
    QString searchPath = parser.value(searchPathOption);
    bool useIndex = parser.isSet(indexOption);
    
    // 创建主窗口
    MainWindow mainWindow;
    mainWindow.setSearchType(searchType);
    mainWindow.setSearchPath(searchPath);
    mainWindow.setUseIndex(useIndex);
    mainWindow.show();
    
    int result = app.exec();
    
    // 关闭搜索库
    DFM::Search::shutdown();
    
    return result;
} 