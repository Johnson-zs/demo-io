#include "workerprocess.h"
#include "../core/connection.h"
#include <QCoreApplication>
#include <QLibrary>
#include <QDebug>
#include <QEventLoop>

namespace DFM {

class WorkerProcess::Private {
public:
    std::unique_ptr<Connection> connection;
    std::unique_ptr<WorkerPlugin> plugin;
    QString pluginPath;
    QString connectionName;
    bool shouldExit = false;
};

WorkerProcess::WorkerProcess(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

WorkerProcess::~WorkerProcess()
{
    // 清理插件
    if (d->plugin) {
        d->plugin->cleanup();
    }
    
    // 关闭连接
    if (d->connection) {
        d->connection->close();
    }
}

bool WorkerProcess::init(const QString &pluginPath, const QString &connectionName)
{
    d->pluginPath = pluginPath;
    d->connectionName = connectionName;
    
    // 加载插件
    if (!loadPlugin(pluginPath)) {
        return false;
    }
    
    // 创建连接
    d->connection = std::make_unique<Connection>();
    
    // 连接信号
    connect(d->connection.get(), &Connection::commandReceived, 
           this, &WorkerProcess::onCommandReceived);
    connect(d->connection.get(), &Connection::error, 
           this, &WorkerProcess::onConnectionError);
    connect(d->connection.get(), &Connection::disconnected, 
           this, &WorkerProcess::onDisconnected);
    
    // 连接到服务器
    if (!d->connection->connectToServer(connectionName)) {
        qCritical() << "Failed to connect to server:" << connectionName;
        return false;
    }
    
    // 初始化插件
    if (!d->plugin->init()) {
        qCritical() << "Failed to initialize plugin:" << pluginPath;
        return false;
    }
    
    // 发送连接成功消息
    d->connection->send(Commands::CMD_CONNECTED);
    
    return true;
}

int WorkerProcess::exec()
{
    // 事件循环
    QEventLoop loop;
    
    // 当连接断开或需要退出时，退出事件循环
    connect(d->connection.get(), &Connection::disconnected, &loop, &QEventLoop::quit);
    connect(this, &QObject::destroyed, &loop, &QEventLoop::quit);
    
    // 如果已经标记为退出，直接返回
    if (d->shouldExit) {
        return 0;
    }
    
    // 执行事件循环
    return loop.exec();
}

bool WorkerProcess::loadPlugin(const QString &pluginPath)
{
    QLibrary library(pluginPath);
    
    if (!library.load()) {
        qCritical() << "Failed to load plugin:" << pluginPath << "-" << library.errorString();
        return false;
    }
    
    // 查找创建函数
    auto createFunction = reinterpret_cast<CreateWorkerPluginFunc>(
        library.resolve("createWorkerPlugin"));
    
    if (!createFunction) {
        qCritical() << "Failed to resolve createWorkerPlugin function in plugin:" << pluginPath;
        library.unload();
        return false;
    }
    
    // 创建插件实例
    WorkerPlugin *plugin = createFunction();
    if (!plugin) {
        qCritical() << "Failed to create plugin instance from:" << pluginPath;
        library.unload();
        return false;
    }
    
    d->plugin.reset(plugin);
    
    return true;
}

void WorkerProcess::onCommandReceived(int cmd, const QByteArray &data)
{
    // 检查系统命令
    if (cmd == Commands::CMD_QUIT) {
        // 收到退出命令
        d->shouldExit = true;
        
        // 处理退出前的清理
        if (d->plugin) {
            d->plugin->cleanup();
        }
        
        // 发送确认退出的消息
        d->connection->send(Commands::CMD_DISCONNECT);
        
        // 关闭连接
        d->connection->close();
        
        // 退出事件循环
        QCoreApplication::exit(0);
        return;
    }
    
    // 调用插件处理命令
    if (d->plugin) {
        d->plugin->handleCommand(cmd, data, 
            [this](int responseCmd, const QByteArray &responseData) {
                // 将插件响应发送回客户端
                if (d->connection) {
                    d->connection->send(responseCmd, responseData);
                }
            });
    } else {
        // 无法处理命令，返回错误
        QByteArray errorData;
        QDataStream stream(&errorData, QIODevice::WriteOnly);
        stream << QString("Plugin not loaded or initialized");
        
        d->connection->send(Commands::MSG_ERROR, errorData);
    }
}

void WorkerProcess::onConnectionError(const QString &errorString)
{
    qWarning() << "Connection error:" << errorString;
    
    // 如果连接出错，准备退出
    d->shouldExit = true;
    QCoreApplication::exit(1);
}

void WorkerProcess::onDisconnected()
{
    qDebug() << "Connection closed";
    
    // 连接断开，准备退出
    d->shouldExit = true;
    QCoreApplication::exit(0);
}

void WorkerProcess::sendResponse(int cmd, const QByteArray &data)
{
    // 将响应发送给客户端
    if (d->connection) {
        d->connection->send(cmd, data);
    }
}

} // namespace DFM 