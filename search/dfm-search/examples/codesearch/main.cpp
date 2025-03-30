#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DFM Search Demo");
    app.setOrganizationName("DFM");
    app.setOrganizationDomain("dfm.org");
    
    MainWindow window;
    window.show();
    
    return app.exec();
} 