#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("File Copy Algorithm Validation Framework");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CopyFiles Demo");
    app.setApplicationDisplayName("File Copy Demo");
    
    // Set a modern style if available
    if (QStyleFactory::keys().contains("Fusion")) {
        app.setStyle("Fusion");
        
        // Set a modern light palette
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, Qt::white);
        lightPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
        lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
        lightPalette.setColor(QPalette::ToolTipText, Qt::black);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, QColor(225, 225, 225));
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::red);
        lightPalette.setColor(QPalette::Link, QColor(0, 100, 200));
        lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
        lightPalette.setColor(QPalette::HighlightedText, Qt::white);
        app.setPalette(lightPalette);
    }
    
    try {
        // Create and show main window
        MainWindow window;
        window.show();
        
        return app.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Fatal Error", 
                             QString("An error occurred: %1").arg(e.what()));
        return 1;
    } catch (...) {
        QMessageBox::critical(nullptr, "Fatal Error", 
                             "An unknown error occurred.");
        return 1;
    }
} 