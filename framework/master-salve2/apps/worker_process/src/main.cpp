#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>
#include <QLocalSocket>
#include <framework/worker_base.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("worker-process");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Worker process for distributed tasks");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加必要参数
    QCommandLineOption pluginPathOption(QStringList() << "p" << "plugin-path",
        "Path to worker plugin", "plugin-path");
    QCommandLineOption serverNameOption(QStringList() << "s" << "server-name",
        "Name of local server", "server-name");
    QCommandLineOption workerIdOption(QStringList() << "i" << "worker-id",
        "Worker ID", "worker-id");
    
    parser.addOption(pluginPathOption);
    parser.addOption(serverNameOption);
    parser.addOption(workerIdOption);
    
    parser.process(app);
    
    // 验证参数
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
    
    // 获取插件实例
    QObject *instance = loader.instance();
    if (!instance) {
        qCritical() << "Failed to get plugin instance";
        return 1;
    }
    
    Framework::WorkerBase *worker = qobject_cast<Framework::WorkerBase*>(instance);
    if (!worker) {
        qCritical() << "Plugin is not a valid worker";
        loader.unload();
        return 1;
    }
    
    // 设置worker ID
    worker->setWorkerId(workerId);
    
    // 创建本地socket并连接到服务器
    QLocalSocket *socket = new QLocalSocket();
    socket->connectToServer(serverName);
    
    if (!socket->waitForConnected(5000)) {
        qCritical() << "Failed to connect to local server:" << socket->errorString();
        loader.unload();
        delete socket;
        return 1;
    }
    
    qInfo() << "Connected to server:" << serverName;
    
    // 初始化worker
    if (!worker->initialize()) {
        qCritical() << "Failed to initialize worker";
        socket->disconnectFromServer();
        loader.unload();
        delete socket;
        return 1;
    }
    
    // 设置worker的消息处理器以通过socket发送消息
    worker->setMessageHandler([socket](const QByteArray &message) {
        socket->write(message);
        socket->flush();
    });
    
    // 处理来自socket的消息
    QObject::connect(socket, &QLocalSocket::readyRead, [socket, worker]() {
        while (socket->bytesAvailable() > 0) {
            QByteArray data = socket->readAll();
            
            // 处理收到的消息
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
    
    // 发送worker注册消息
    QByteArray registerMessage;
    QDataStream stream(&registerMessage, QIODevice::WriteOnly);
    stream << QString("REGISTER") << workerId << worker->capabilities();
    socket->write(registerMessage);
    socket->flush();
    
    qInfo() << "Worker registered with capabilities:" << worker->capabilities();
    
    int result = app.exec();
    
    // 清理资源
    worker->terminate();
    socket->disconnectFromServer();
    loader.unload();
    delete socket;
    
    return result;
} 