#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUuid>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QTimer>

#include <framework/message.h>
#include <framework/common.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test-client");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("测试主机客户端程序，用于测试Worker进程");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加命令行选项
    QCommandLineOption workerProcessOption(QStringList() << "w" << "worker-process",
        "Worker进程可执行文件路径", "worker-process", "bin/worker_process");
    
    QCommandLineOption workerPluginOption(QStringList() << "p" << "worker-plugin",
        "Worker插件路径", "worker-plugin", "workers/libimage_processor.so");
    
    parser.addOption(workerProcessOption);
    parser.addOption(workerPluginOption);
    
    parser.process(app);
    
    QString workerProcess = parser.value(workerProcessOption);
    QString workerPlugin = parser.value(workerPluginOption);
    
    qInfo() << "Worker进程路径:" << workerProcess;
    qInfo() << "Worker插件路径:" << workerPlugin;
    
    // 创建本地服务器
    QString serverName = "test_server_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QLocalServer server;
    
    if (!server.listen(serverName)) {
        qCritical() << "无法启动本地服务器:" << server.errorString();
        return 1;
    }
    
    qInfo() << "本地服务器已启动:" << serverName;
    
    // 启动Worker进程
    QProcess workerProc;
    QString workerId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QStringList args;
    args << "--plugin-path" << workerPlugin
         << "--server-name" << serverName
         << "--worker-id" << workerId;
    
    qInfo() << "启动Worker进程...";
    qInfo() << workerProcess << args.join(" ");
    
    workerProc.start(workerProcess, args);
    
    if (!workerProc.waitForStarted(5000)) {
        qCritical() << "无法启动Worker进程:" << workerProc.errorString();
        return 1;
    }
    
    // Worker连接
    QLocalSocket *socket = nullptr;
    
    QObject::connect(&server, &QLocalServer::newConnection, [&server, &socket]() {
        socket = server.nextPendingConnection();
        qInfo() << "Worker已连接!";
        
        // 处理Worker发送的消息
        QObject::connect(socket, &QLocalSocket::readyRead, [socket]() {
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
                    Framework::RegisterMessage regMsg;
                    stream >> regMsg;
                    
                    qInfo() << "Worker注册:" << regMsg.workerId;
                    qInfo() << "能力:" << regMsg.capabilities.join(", ");
                    break;
                }
                case Framework::MessageType::TASK_STATUS: {
                    Framework::TaskStatusMessage status;
                    stream >> status;
                    
                    qInfo() << "任务状态更新:";
                    qInfo() << "  任务ID:" << status.taskId;
                    qInfo() << "  Worker ID:" << status.workerId;
                    qInfo() << "  状态:" << static_cast<int>(status.status);
                    qInfo() << "  进度:" << status.progress << "%";
                    if (!status.statusMessage.isEmpty()) {
                        qInfo() << "  消息:" << status.statusMessage;
                    }
                    break;
                }
                case Framework::MessageType::TASK_RESULT: {
                    Framework::TaskResultMessage result;
                    stream >> result;
                    
                    qInfo() << "任务结果:";
                    qInfo() << "  任务ID:" << result.taskId;
                    qInfo() << "  Worker ID:" << result.workerId;
                    qInfo() << "  成功:" << (result.success ? "是" : "否");
                    qInfo() << "  结果:" << result.result.toMap();
                    break;
                }
                default: {
                    qWarning() << "收到未知消息类型:" << typeInt;
                    break;
                }
            }
        });
    });
    
    // 等待Worker连接
    QTimer connectionTimer;
    connectionTimer.setSingleShot(true);
    
    QObject::connect(&connectionTimer, &QTimer::timeout, [&app]() {
        qCritical() << "等待Worker连接超时";
        app.quit();
    });
    
    connectionTimer.start(10000);
    
    // 发送任务定时器
    QTimer *taskTimer = new QTimer();
    
    QObject::connect(taskTimer, &QTimer::timeout, [&socket, &taskTimer]() {
        if (!socket || socket->state() != QLocalSocket::ConnectedState) {
            qWarning() << "Worker未连接，无法发送任务";
            return;
        }
        
        // 创建任务
        Framework::TaskMessage task;
        task.taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        task.taskType = "ImageProcessing";
        
        // 任务参数
        task.parameters["input_file"] = "test.jpg";
        task.parameters["output_file"] = "output.jpg";
        task.parameters["operation"] = "resize";
        task.parameters["width"] = 800;
        task.parameters["height"] = 600;
        
        // 序列化任务
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << task;
        
        // 发送任务
        socket->write(data);
        socket->flush();
        
        qInfo() << "已发送任务:" << task.taskId;
        
        // 停止任务定时器，只发送一次任务
        taskTimer->stop();
    });
    
    // 在连接成功后启动任务发送定时器
    QObject::connect(&server, &QLocalServer::newConnection, [&taskTimer]() {
        // 等待2秒后发送任务
        taskTimer->start(2000);
    });
    
    // 清理资源
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&workerProc, &server, &socket]() {
        if (socket && socket->state() == QLocalSocket::ConnectedState) {
            // 发送关闭命令
            Framework::Message shutdownMsg(Framework::MessageType::SHUTDOWN);
            
            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            stream << shutdownMsg;
            
            socket->write(data);
            socket->flush();
            
            // 等待关闭
            socket->waitForBytesWritten(1000);
            socket->disconnectFromServer();
        }
        
        // 终止Worker进程
        if (workerProc.state() != QProcess::NotRunning) {
            workerProc.terminate();
            if (!workerProc.waitForFinished(3000)) {
                workerProc.kill();
            }
        }
        
        server.close();
    });
    
    // 设置超时退出
    QTimer::singleShot(30000, &app, &QCoreApplication::quit);
    
    return app.exec();
} 