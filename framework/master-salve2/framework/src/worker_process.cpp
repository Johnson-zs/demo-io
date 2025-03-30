#include "framework/worker_factory.h"
#include "framework/worker_base.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QPluginLoader>
#include <QLocalSocket>
#include <QDataStream>
#include <QDir>
#include <QDebug>
#include <QHostInfo>

using namespace Framework;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("worker-process");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Worker process for KIO-style worker framework");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 定义命令行选项
    QCommandLineOption pluginPathOption(QStringList() << "p" << "plugin-path",
        "Path to worker plugin", "plugin-path");
    QCommandLineOption serverNameOption(QStringList() << "s" << "server-name",
        "Local server name for IPC", "server-name");
    QCommandLineOption workerIdOption(QStringList() << "i" << "worker-id",
        "Worker ID", "worker-id");
    
    parser.addOption(pluginPathOption);
    parser.addOption(serverNameOption);
    parser.addOption(workerIdOption);
    
    parser.process(app);
    
    // 检查必要的选项
    if (!parser.isSet(pluginPathOption) || !parser.isSet(serverNameOption) || !parser.isSet(workerIdOption)) {
        parser.showHelp(1);
        return 1;
    }
    
    QString pluginPath = parser.value(pluginPathOption);
    QString serverName = parser.value(serverNameOption);
    QString workerId = parser.value(workerIdOption);
    
    qInfo() << "Starting worker process with ID:" << workerId;
    qInfo() << "Plugin path:" << pluginPath;
    qInfo() << "Server name:" << serverName;
    
    // 加载插件
    QPluginLoader loader(pluginPath);
    if (!loader.load()) {
        qCritical() << "Failed to load plugin:" << loader.errorString();
        return 1;
    }
    
    // 获取插件工厂
    QObject *instance = loader.instance();
    if (!instance) {
        qCritical() << "Failed to get plugin instance";
        loader.unload();
        return 1;
    }
    
    WorkerFactory *factory = qobject_cast<WorkerFactory*>(instance);
    if (!factory) {
        qCritical() << "Plugin is not a valid worker factory";
        loader.unload();
        return 1;
    }
    
    // 创建Worker实例
    WorkerBase *worker = factory->createWorker();
    if (!worker) {
        qCritical() << "Failed to create worker instance";
        loader.unload();
        return 1;
    }
    
    // 设置Worker ID
    worker->setWorkerId(workerId);
    
    // 连接到本地服务器
    QLocalSocket *socket = new QLocalSocket();
    socket->connectToServer(serverName);
    
    if (!socket->waitForConnected(5000)) {
        qCritical() << "Failed to connect to server:" << socket->errorString();
        delete worker;
        loader.unload();
        delete socket;
        return 1;
    }
    
    qInfo() << "Connected to server:" << serverName;
    
    // 初始化worker
    if (!worker->initialize()) {
        qCritical() << "Failed to initialize worker";
        socket->disconnectFromServer();
        delete worker;
        loader.unload();
        delete socket;
        return 1;
    }
    
    // 设置消息处理器
    worker->setMessageHandler([socket](const QByteArray &message) {
        socket->write(message);
        socket->flush();
    });
    
    // 处理来自socket的消息
    QObject::connect(socket, &QLocalSocket::readyRead, [socket, worker]() {
        while (socket->bytesAvailable() > 0) {
            QByteArray data = socket->readAll();
            if (!worker->handleMessage(data)) {
                qWarning() << "Failed to handle message";
            }
        }
    });
    
    // 处理socket断开连接
    QObject::connect(socket, &QLocalSocket::disconnected, [&app]() {
        qInfo() << "Disconnected from server, exiting...";
        app.quit();
    });
    
    // 发送注册消息，包含Worker支持的任务类型
    QByteArray registerMessage;
    QDataStream stream(&registerMessage, QIODevice::WriteOnly);
    stream << static_cast<int>(MessageType::REGISTER);
    
    RegisterMessage regMsg;
    regMsg.workerId = workerId;
    regMsg.hostname = QHostInfo::localHostName();
    regMsg.capabilities = factory->supportedTaskTypes();
    
    stream.device()->seek(0);
    stream << regMsg;
    
    socket->write(registerMessage);
    socket->flush();
    
    qInfo() << "Worker registered with capabilities:" << factory->supportedTaskTypes();
    
    int result = app.exec();
    
    // 清理资源
    worker->terminate();
    socket->disconnectFromServer();
    delete worker;
    loader.unload();
    delete socket;
    
    return result;
} 