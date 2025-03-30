#include <QCoreApplication>
#include "slave.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    Slave slave;
    slave.start();
    
    return app.exec();
} 