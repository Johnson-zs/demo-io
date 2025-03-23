#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QDir>
#include <QTimer>

#include "scheduler.h"
#include "simplejob.h"
#include "job.h"
#include "workerinterface.h"

using namespace DFM;

class TestApp : public QObject
{
    Q_OBJECT

public:
    TestApp(QObject *parent = nullptr)
        : QObject(parent)
        , m_job(nullptr)
    {
    }

    void start(const QString &path)
    {
        qDebug() << "测试开始，路径:" << path;

        // 创建SimpleJob，用于获取目录大小
        m_job = SimpleJob::stat(QUrl::fromLocalFile(path));

        // 连接信号
        connect(m_job, &Job::result, this, &TestApp::slotResult);
        connect(m_job, &Job::progress, this, &TestApp::slotProgress);
        
        // 启动Job
        m_job->start();
    }

private Q_SLOTS:
    void slotResult(Job *job)
    {
        qDebug() << "测试结果:";
        
        if (job->error()) {
            qDebug() << "  错误:" << job->errorString();
        } else {
            // 从SimpleJob获取统计信息
            SimpleJob *simpleJob = qobject_cast<SimpleJob*>(job);
            if (simpleJob) {
                qDebug() << "  成功获取统计信息:";
                qDebug() << "  文件大小:" << simpleJob->statResult().size << "字节";
                qDebug() << "  文件类型:" << simpleJob->statResult().type;
                qDebug() << "  修改时间:" << simpleJob->statResult().mtime.toString();
            }
        }
        
        // 完成后退出
        QCoreApplication::quit();
    }
    
    void slotProgress(Job *job, qulonglong processed, qulonglong total)
    {
        qDebug() << "进度:" << processed << "/" << total;
    }

private:
    Job *m_job;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    QCoreApplication::setApplicationName("test-du");
    QCoreApplication::setApplicationVersion("1.0");
    
    // 创建命令行解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("DFM DU Worker测试程序");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加参数
    parser.addPositionalArgument("path", "要统计的路径，默认为当前目录");
    
    // 解析命令行参数
    parser.process(app);
    
    // 获取要统计的路径
    QString path = ".";
    if (!parser.positionalArguments().isEmpty()) {
        path = parser.positionalArguments().first();
    }
    
    // 确保路径存在
    if (!QDir(path).exists()) {
        qCritical() << "错误: 指定的路径不存在:" << path;
        return 1;
    }
    
    // 初始化Scheduler
    Scheduler::self();
    
    // 创建测试应用
    TestApp testApp;
    
    // 延迟启动，给Worker一些时间启动
    QTimer::singleShot(1000, [&testApp, path]() {
        testApp.start(path);
    });
    
    return app.exec();
}

#include "test-du.moc" 