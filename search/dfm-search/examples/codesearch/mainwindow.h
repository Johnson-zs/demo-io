#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QStatusBar>
#include <memory>

// 移除searchtypes.h的包含，改为前向声明命名空间和类型
namespace DFM {
namespace Search {
    class SearchEngineManager;
    enum class SearchType;
}
}

class SearchWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void about();
    void onSearchCompleted(DFM::Search::SearchType type, bool success);
    void onSearchError(DFM::Search::SearchType type, const QString& error);

private:
    void setupMenus();
    void setupActions();
    void setupToolbar();
    void setupStatusBar();
    void setupCentralWidget();
    
private:
    // UI元素
    QToolBar *m_toolbar;
    QTabWidget *m_tabWidget;
    QStatusBar *m_statusBar;
    
    // 菜单和动作
    QMenu *m_fileMenu;
    QMenu *m_searchMenu;
    QMenu *m_helpMenu;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_clearAction;
    
    // 搜索组件
    SearchWidget *m_filenameSearchWidget;
    SearchWidget *m_contentSearchWidget;
    
    // 搜索引擎管理器
    std::shared_ptr<DFM::Search::SearchEngineManager> m_searchManager;
};

#endif // MAINWINDOW_H 
