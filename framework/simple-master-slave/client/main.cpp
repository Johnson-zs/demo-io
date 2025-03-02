#include <QCoreApplication>
#include "client.h"
#include <QTimer>
#include <QTime>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    Client client;
    
    // 示例：每2秒发送一条消息
    QTimer *timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, [&client]() {
        client.sendMessage("Hello from client! " + QString::number(QTime::currentTime().msec()));
    });
    timer->start(2000);
    
    return app.exec();
} 
