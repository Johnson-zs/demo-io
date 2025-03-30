#include <QCoreApplication>
#include <QUuid>

#include "master.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Master master;
    master.start();

    QTimer::singleShot(5000, [&master]() {
        // 创建示例任务
        TaskMessage task;
        task.taskId = QUuid::createUuid().toString();
        task.taskType = "ImageProcessing";
        task.priority = 1;
        task.deadline = QDateTime::currentDateTime().addSecs(3600);
        task.parameters = {
            { "input_file", "image.jpg" },
            { "operation", "resize" },
            { "width", 800 },
            { "height", 600 }
        };

        master.submitTask(task);
    });

    return app.exec();
}
