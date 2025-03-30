#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QDir>
#include <QUuid>
#include <QDebug>

#include "framework/message.h"
#include "framework/common.h"

// 存储worker信息
struct WorkerInfo {
    QString workerId;
    QString hostname;
    QDateTime lastHeartbeat;
    QTcpSocket *socket;
    QStringList capabilities;
    QStringList runningTasks;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Example");
    app.setApplicationVersion(Framework::Version::toString());
    
    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("Example application for the master-slave-demo framework");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 端口选项
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Port to listen on",
                                  "port", 
                                  QString::number(Framework::Constants::DEFAULT_PORT));
    parser.addOption(portOption);
    
    // worker路径选项
    QCommandLineOption workerPathOption(QStringList() << "w" << "worker-path",
                                        "Path to worker executable",
                                        "path", 
                                        QDir::currentPath() + "/worker");
    parser.addOption(workerPathOption);
    
    // 插件目录选项
    QCommandLineOption pluginDirOption(QStringList() << "d" << "plugin-dir",
                                       "Directory containing plugins",
                                       "directory", 
                                       QDir::currentPath() + "/plugins");
    parser.addOption(pluginDirOption);
    
    // 解析
    parser.process(app);
    
    // 获取配置
    int port = parser.value(portOption).toInt();
    QString workerPath = parser.value(workerPathOption);
    QString pluginDir = parser.value(pluginDirOption);
    
    qInfo() << "Example application starting up...";
    qInfo() << "Listening on port:" << port;
    qInfo() << "Worker path:" << workerPath;
    qInfo() << "Plugin directory:" << pluginDir;
    
    // 创建服务器
    QTcpServer server;
    QHash<QString, WorkerInfo> workers;
    
    // 启动服务器
    if (!server.listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server:" << server.errorString();
        return 1;
    }
    
    qInfo() << "Server started on port" << port;
    
    // 处理新连接
    QObject::connect(&server, &QTcpServer::newConnection, [&server, &workers]() {
        QTcpSocket *socket = server.nextPendingConnection();
        
        qInfo() << "New connection from" << socket->peerAddress().toString();
        
        // 处理数据
        QObject::connect(socket, &QTcpSocket::readyRead, [socket, &workers]() {
            QByteArray data = socket->readAll();
            QDataStream stream(data);
            
            // 读取消息类型
            int typeInt;
            stream >> typeInt;
            Framework::MessageType type = static_cast<Framework::MessageType>(typeInt);
            
            // 重置流位置
            stream.device()->seek(0);
            
            // 根据消息类型处理
            switch (type) {
                case Framework::MessageType::REGISTER: {
                    Framework::RegisterMessage reg;
                    stream >> reg;
                    
                    // 添加worker信息
                    WorkerInfo info;
                    info.workerId = reg.workerId;
                    info.hostname = reg.hostname;
                    info.lastHeartbeat = QDateTime::currentDateTime();
                    info.socket = socket;
                    info.capabilities = reg.capabilities;
                    
                    workers[reg.workerId] = info;
                    
                    qInfo() << "Worker registered:" << reg.workerId 
                            << "Hostname:" << reg.hostname
                            << "Capabilities:" << reg.capabilities.join(", ");
                    
                    break;
                }
                case Framework::MessageType::HEARTBEAT: {
                    Framework::HeartbeatMessage heartbeat;
                    stream >> heartbeat;
                    
                    if (workers.contains(heartbeat.workerId)) {
                        workers[heartbeat.workerId].lastHeartbeat = QDateTime::currentDateTime();
                        workers[heartbeat.workerId].runningTasks = heartbeat.runningTaskIds;
                        
                        qDebug() << "Heartbeat from worker:" << heartbeat.workerId
                                 << "CPU:" << heartbeat.cpuUsage << "%"
                                 << "Memory:" << (heartbeat.memoryUsage / 1024 / 1024) << "MB";
                    }
                    
                    break;
                }
                case Framework::MessageType::TASK_STATUS: {
                    Framework::TaskStatusMessage status;
                    stream >> status;
                    
                    qInfo() << "Task status update:"
                            << "Task:" << status.taskId
                            << "Worker:" << status.workerId
                            << "Status:" << static_cast<int>(status.status)
                            << "Progress:" << status.progress << "%"
                            << "Message:" << status.statusMessage;
                    
                    break;
                }
                case Framework::MessageType::TASK_RESULT: {
                    Framework::TaskResultMessage result;
                    stream >> result;
                    
                    qInfo() << "Task result:"
                            << "Task:" << result.taskId
                            << "Worker:" << result.workerId
                            << "Success:" << (result.success ? "Yes" : "No")
                            << "Result:" << result.result.toString();
                    
                    break;
                }
                default: {
                    qWarning() << "Received unknown message type:" << typeInt;
                    break;
                }
            }
        });
        
        // 处理断开连接
        QObject::connect(socket, &QTcpSocket::disconnected, [socket, &workers]() {
            qInfo() << "Connection closed from" << socket->peerAddress().toString();
            
            // 移除worker
            QString workerId;
            for (auto it = workers.begin(); it != workers.end(); ++it) {
                if (it.value().socket == socket) {
                    workerId = it.key();
                    break;
                }
            }
            
            if (!workerId.isEmpty()) {
                workers.remove(workerId);
                qInfo() << "Worker removed:" << workerId;
            }
            
            socket->deleteLater();
        });
    });
    
    // 启动一个worker进程
    qInfo() << "Starting worker process...";
    QProcess workerProcess;
    workerProcess.setProgram(workerPath);
    workerProcess.setArguments({
        "--plugin-dir", pluginDir,
        "--host", "localhost",
        "--port", QString::number(port)
    });
    
    workerProcess.start();
    
    // 处理worker进程退出
    QObject::connect(&workerProcess, &QProcess::errorOccurred, [&workerProcess](QProcess::ProcessError error) {
        qWarning() << "Worker process error:" << error;
    });
    
    // 在应用程序退出时终止worker
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&workerProcess, &workers, &server]() {
        qInfo() << "Shutting down...";
        
        // 发送关闭命令给所有worker
        for (const auto &worker : workers) {
            if (worker.socket && worker.socket->state() == QTcpSocket::ConnectedState) {
                Framework::Message shutdownMsg(Framework::MessageType::SHUTDOWN);
                
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << shutdownMsg;
                
                worker.socket->write(data);
                worker.socket->flush();
            }
        }
        
        // 等待worker关闭
        QThread::msleep(500);
        
        // 终止worker进程
        if (workerProcess.state() != QProcess::NotRunning) {
            workerProcess.terminate();
            if (!workerProcess.waitForFinished(3000)) {
                workerProcess.kill();
            }
        }
        
        server.close();
    });
    
    // 在1秒后发送一个示例任务
    QTimer::singleShot(1000, [&workers]() {
        // 检查是否有worker可用
        if (workers.isEmpty()) {
            qWarning() << "No workers available to send task";
            return;
        }
        
        // 获取第一个worker
        const WorkerInfo &worker = *workers.begin();
        
        // 检查是否支持ImageProcessing任务
        if (!worker.capabilities.contains("ImageProcessing")) {
            qWarning() << "Worker does not support ImageProcessing task";
            return;
        }
        
        // 创建任务消息
        Framework::TaskMessage task;
        task.taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        task.workerId = worker.workerId;
        task.taskType = "ImageProcessing";
        
        // 任务参数
        task.parameters["input_file"] = "example.jpg";
        task.parameters["operation"] = "resize";
        task.parameters["width"] = 800;
        task.parameters["height"] = 600;
        
        // 发送任务
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << task;
        
        worker.socket->write(data);
        
        qInfo() << "Sent ImageProcessing task:" << task.taskId << "to worker:" << worker.workerId;
    });
    
    return app.exec();
} 