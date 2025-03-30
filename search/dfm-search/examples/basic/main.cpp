#include "mainwindow.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DFM Search Example");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("DFM");
    app.setOrganizationDomain("deepin.org");
    
    // 设置图标主题
    QIcon::setThemeName("hicolor");
    
    // 设置当前目录为应用程序所在目录
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
} 