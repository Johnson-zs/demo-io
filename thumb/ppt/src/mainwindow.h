#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QProgressBar>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>

class ThumbnailGenerator;
class ThumbnailWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void clearThumbnails();
    void onThumbnailGenerated(const QString &imagePath, int pageNumber);
    void onGenerationFinished();
    void onGenerationError(const QString &error);
    void showAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void updateStatusMessage(const QString &message);

    // UI组件
    QWidget *m_centralWidget;
    QSplitter *m_splitter;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_toolbarLayout;
    
    QPushButton *m_openButton;
    QPushButton *m_clearButton;
    QLabel *m_fileInfoLabel;
    QLabel *m_pageCountLabel;
    
    QScrollArea *m_scrollArea;
    QWidget *m_thumbnailContainer;
    QGridLayout *m_thumbnailLayout;
    
    QProgressBar *m_progressBar;
    
    // 状态栏
    QLabel *m_statusLabel;
    QLabel *m_pageLabel;
    
    // 业务逻辑
    ThumbnailGenerator *m_generator;
    QString m_currentFile;
    int m_totalPages;
    int m_currentPage;
    
    // 缩略图列表
    QList<ThumbnailWidget*> m_thumbnailWidgets;
};

#endif // MAINWINDOW_H 