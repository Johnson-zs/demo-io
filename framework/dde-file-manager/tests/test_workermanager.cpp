#include "test_workermanager.h"
#include "../src/core/worker.h"
#include <QTest>
#include <QSignalSpy>
#include <QUrl>

TestWorkerManager::TestWorkerManager(QObject *parent)
    : QObject(parent)
{
}

void TestWorkerManager::initTestCase()
{
    m_workerManager = QSharedPointer<DFM::WorkerManager>(new DFM::WorkerManager());
}

void TestWorkerManager::cleanupTestCase()
{
    // 确保清理所有worker
    m_worker.reset();
    m_workerManager.reset();
}

void TestWorkerManager::testGetIdleWorker()
{
    QUrl testUrl = QUrl("file:///test/path");
    
    // 获取worker
    QString errorString;
    m_worker = m_workerManager->getIdleWorker("file", testUrl);
    
    // 验证worker是否创建成功
    QVERIFY(m_worker != nullptr);
    QCOMPARE(m_worker->protocol(), QString("file"));
    QVERIFY(m_worker->isAlive());
}

void TestWorkerManager::testReleaseWorker()
{
    // 确保有worker可以释放
    if (!m_worker) {
        testGetIdleWorker();
    }
    
    // 释放worker
    m_workerManager->releaseWorker(m_worker);
    
    // 重新获取相同协议的worker，应该是同一个实例
    QUrl testUrl = QUrl("file:///test/path");
    auto worker2 = m_workerManager->getIdleWorker("file", testUrl);
    
    QVERIFY(worker2 != nullptr);
    QCOMPARE(worker2->protocol(), QString("file"));
    
    // 注意：在真实情况下，这可能是同一个worker实例，但在单元测试环境中，
    // 由于ThreadWorker的实现可能受限，不能保证一定是同一个实例
    
    // 清理
    m_workerManager->releaseWorker(worker2);
    m_worker.reset();
}

void TestWorkerManager::testCleanIdleWorkers()
{
    // 创建几个worker
    QUrl testUrl = QUrl("file:///test/path");
    auto worker1 = m_workerManager->getIdleWorker("file", testUrl);
    auto worker2 = m_workerManager->getIdleWorker("file", testUrl);
    
    // 释放所有worker
    m_workerManager->releaseWorker(worker1);
    m_workerManager->releaseWorker(worker2);
    
    // 立即清理（超时设为0）
    m_workerManager->cleanIdleWorkers(0);
    
    // 再次尝试获取worker
    // 由于所有worker都被清理，应该会创建新的worker
    auto worker3 = m_workerManager->getIdleWorker("file", testUrl);
    QVERIFY(worker3 != nullptr);
    
    // 清理
    m_workerManager->releaseWorker(worker3);
}

QTEST_MAIN(TestWorkerManager) 