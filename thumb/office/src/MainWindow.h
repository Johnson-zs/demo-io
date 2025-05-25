#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

class OfficePreviewWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void openFile();
    void showAbout();
    void onFileOpened(const QString &filePath);
    void onPreviewProgress(int percentage);
    void onPreviewError(const QString &error);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void updateRecentFiles(const QString &filePath);
    void loadRecentFiles();
    bool isOfficeFile(const QString &filePath) const;

    // UI组件
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QTreeWidget *m_recentFilesTree;
    OfficePreviewWidget *m_previewWidget;
    
    // 工具栏和状态栏
    QPushButton *m_openButton;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    
    // 菜单
    QMenu *m_fileMenu;
    QMenu *m_helpMenu;
    QAction *m_openAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    
    // 数据
    QStringList m_recentFiles;
    static const int MAX_RECENT_FILES = 10;
}; 