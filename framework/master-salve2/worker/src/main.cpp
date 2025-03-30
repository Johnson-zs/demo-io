#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>

#include "worker/worker.h"
#include "framework/common.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Worker");
    app.setApplicationVersion(Framework::Version::toString());
    
    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("Worker process for the master-slave-demo framework");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 插件目录选项
    QCommandLineOption pluginDirOption(QStringList() << "p" << "plugin-dir",
                                       "Directory containing plugins",
                                       "directory", 
                                      #ifdef PLUGINS_DIR
                                       PLUGINS_DIR
                                      #else
                                       QDir::currentPath() + "/plugins"
                                      #endif
                                       );
    parser.addOption(pluginDirOption);
    
    // 主服务器地址选项
    QCommandLineOption masterHostOption(QStringList() << "h" << "host",
                                        "Master server host",
                                        "host", "localhost");
    parser.addOption(masterHostOption);
    
    // 主服务器端口选项
    QCommandLineOption masterPortOption(QStringList() << "P" << "port",
                                        "Master server port",
                                        "port", 
                                        QString::number(Framework::Constants::DEFAULT_PORT));
    parser.addOption(masterPortOption);
    
    // 解析
    parser.process(app);
    
    // 获取配置
    QString pluginDir = parser.value(pluginDirOption);
    QString masterHost = parser.value(masterHostOption);
    int masterPort = parser.value(masterPortOption).toInt();
    
    qInfo() << "Worker starting up...";
    qInfo() << "Plugin directory:" << pluginDir;
    qInfo() << "Master server:" << masterHost << ":" << masterPort;
    
    // 创建worker对象
    Worker worker;
    
    // 启动worker
    if (!worker.start(pluginDir, masterHost, masterPort)) {
        qWarning() << "Worker failed to start properly.";
        // 继续运行，因为worker会尝试重连
    }
    
    return app.exec();
} 