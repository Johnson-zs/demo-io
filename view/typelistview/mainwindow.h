#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QToolBar>
#include <QStatusBar>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <memory>
#include <QLineEdit>

// Forward declarations
class FileSystemModel;
class FileListView;
class ContextMenuController;

/**
 * @brief Main window of the file browser application
 * 
 * This class provides the main user interface including the file list view,
 * toolbar with navigation and view controls, and status bar.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDirectoryChanged(const QString& path);
    void onLoadingStarted();
    void onLoadingFinished();
    void onSortingChanged();
    void onGroupingChanged();
    void onPathEditingFinished();
    void onUpDirectoryTriggered();

private:
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void updateStatusBar();
    void updatePathEdit(const QString& path);
    void navigateToPath(const QString& path);

private:
    // Core components
    FileSystemModel* m_model;
    FileListView* m_listView;
    ContextMenuController* m_contextMenuController;
    
    // UI components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QToolBar* m_toolBar;
    QComboBox* m_sortingCombo;
    QComboBox* m_groupingCombo;
    QLineEdit* m_pathEdit;
    QAction* m_refreshAction;
    QAction* m_homeAction;
    QAction* m_upAction;
    
    // Status bar components
    QLabel* m_pathLabel;
    QLabel* m_itemCountLabel;
    QLabel* m_loadingLabel;
    
    // Current state
    QString m_currentPath;
};

#endif // MAINWINDOW_H
