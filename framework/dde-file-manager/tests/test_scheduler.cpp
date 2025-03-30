#include "test_scheduler.h"
#include "../src/core/scheduler.h"
#include "../src/core/job.h"
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QVariant>

// 用于测试的简单任务类
class TestJob : public DFM::SimpleJob {
    Q_OBJECT
public:
    TestJob(const QUrl &url, QObject *parent = nullptr)
        : DFM::SimpleJob(url, parent)
    {
    }
    
    JobType type() const override { return JobType::Generic; }
    
    void start() override
    {
        setState(JobState::Running);
        // 模拟异步操作
        QTimer::singleShot(100, [this]() {
            emit dataReceived(QVariant("Test data"));
            setState(JobState::Finished);
        });
    }
};

TestScheduler::TestScheduler(QObject *parent)
    : QObject(parent)
{
}

void TestScheduler::initTestCase()
{
    // 获取调度器单例
    m_scheduler = DFM::Scheduler::instance();
    QVERIFY(m_scheduler != nullptr);
}

void TestScheduler::cleanupTestCase()
{
    // 等待所有任务完成
    QTest::qWait(200);
}

void TestScheduler::testScheduleJob()
{
    // 监听任务开始和完成信号
    QSignalSpy startedSpy(m_scheduler, &DFM::Scheduler::jobStarted);
    QSignalSpy finishedSpy(m_scheduler, &DFM::Scheduler::jobFinished);
    
    // 创建测试任务
    QUrl testUrl = QUrl("file:///test/path");
    auto job = std::make_shared<TestJob>(testUrl);
    
    // 调度任务
    m_scheduler->scheduleJob(job);
    
    // 等待任务开始
    QVERIFY(startedSpy.wait(1000));
    QVERIFY(startedSpy.count() > 0);
    
    // 等待任务完成
    QVERIFY(finishedSpy.wait(5000));
    QVERIFY(finishedSpy.count() > 0);
    
    // 验证任务结果
    QList<QVariant> arguments = finishedSpy.takeLast();
    QCOMPARE(arguments.at(1).toBool(), true); // 成功完成
}

void TestScheduler::testCancelJob()
{
    // 创建一个可以被取消的测试任务
    QUrl testUrl = QUrl("file:///test/path");
    auto job = std::make_shared<TestJob>(testUrl);
    
    // 监听信号
    QSignalSpy startedSpy(m_scheduler, &DFM::Scheduler::jobStarted);
    QSignalSpy finishedSpy(m_scheduler, &DFM::Scheduler::jobFinished);
    
    // 调度任务
    m_scheduler->scheduleJob(job);
    
    // 等待任务开始
    QVERIFY(startedSpy.wait(1000));
    
    // 立即取消任务
    m_scheduler->cancelJob(job);
    
    // 验证任务状态
    QCOMPARE(job->state(), DFM::JobState::Canceled);
    
    // 验证调度器是否处理了取消
    // 注意：可能不会收到finishedSpy信号，因为任务被取消了
}

void TestScheduler::testMaxWorkers()
{
    // 设置最大并发数
    int maxWorkers = 2;
    m_scheduler->setMaxWorkers(maxWorkers);
    QCOMPARE(m_scheduler->maxWorkers(), maxWorkers);
    
    // 清理所有现有任务
    QTest::qWait(200);
    
    // 创建多个任务
    QSignalSpy startedSpy(m_scheduler, &DFM::Scheduler::jobStarted);
    
    std::vector<std::shared_ptr<TestJob>> jobs;
    for (int i = 0; i < maxWorkers + 2; i++) {
        QUrl testUrl = QUrl(QString("file:///test/path/%1").arg(i));
        auto job = std::make_shared<TestJob>(testUrl);
        jobs.push_back(job);
        m_scheduler->scheduleJob(job);
    }
    
    // 等待一段时间
    QTest::qWait(500);
    
    // 验证只有最大并发数的任务被启动
    QVERIFY(startedSpy.count() <= maxWorkers);
    
    // 等待所有任务完成
    QTest::qWait(1000);
    
    // 重置最大并发数为默认值
    m_scheduler->setMaxWorkers(5);
}

QTEST_MAIN(TestScheduler)
#include "test_scheduler.moc"  // 包含生成的元对象代码 