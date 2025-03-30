#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "workerprocess.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("dde-file-manager-worker");
    app.setApplicationVersion("0.1.0");

    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("DDE File Manager Worker Process");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption pluginOption(QStringList() << "p" << "plugin", 
                                    "Worker plugin to load", "plugin_path");
    QCommandLineOption connectionOption(QStringList() << "c" << "connection", 
                                        "Connection name for IPC", "connection_name");
    
    parser.addOption(pluginOption);
    parser.addOption(connectionOption);
    
    parser.process(app);
    
    // 检查必要参数
    if (!parser.isSet(pluginOption) || !parser.isSet(connectionOption)) {
        qCritical() << "Missing required options. Use --help for more information.";
        return 1;
    }
    
    // 创建工作进程
    DFM::WorkerProcess workerProcess;
    if (!workerProcess.init(parser.value(pluginOption), parser.value(connectionOption))) {
        qCritical() << "Failed to initialize worker process.";
        return 2;
    }
    
    // 运行事件循环
    return workerProcess.exec();
} 