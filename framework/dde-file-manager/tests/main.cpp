#include <QCoreApplication>
#include <QTest>
#include "test_workermanager.h"
#include "test_scheduler.h"
#include "test_connection.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    int status = 0;
    
    // 运行各个测试类
    {
        TestConnection tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    {
        TestWorkerManager twm;
        status |= QTest::qExec(&twm, argc, argv);
    }
    
    {
        TestScheduler ts;
        status |= QTest::qExec(&ts, argc, argv);
    }
    
    return status;
} 