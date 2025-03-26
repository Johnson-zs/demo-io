#include <QCoreApplication>
#include <QCommandLineParser>
#include "search_worker_service.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("qsearch-worker");
    app.setApplicationVersion("1.0.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("QSearch搜索工作进程");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加日志级别选项
    QCommandLineOption logLevelOption(QStringList() << "l" << "log-level",
        "设置日志级别 (debug, info, warning, error)", "level", "info");
    parser.addOption(logLevelOption);
    
    // 解析命令行参数
    parser.process(app);
    
    // 设置日志级别
    QString logLevel = parser.value(logLevelOption);
    // ... 配置日志系统 ...
    
    // 创建并启动工作服务
    SearchWorkerService service;
    if (!service.start()) {
        return 1;
    }
    
    return app.exec();
} 