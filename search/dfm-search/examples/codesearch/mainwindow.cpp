#include "mainwindow.h"
#include "searchwidget.h"

#include "dfm-search/searchengine.h"
#include "dfm-search/filenamesearch.h"
#include "dfm-search/contentsearch.h"

#include <QApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <QHeaderView>
#include <QDir>

using namespace DFM::Search;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_toolbar(nullptr)
    , m_tabWidget(nullptr)
    , m_statusBar(nullptr)
    , m_fileMenu(nullptr)
    , m_searchMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_exitAction(nullptr)
    , m_aboutAction(nullptr)
    , m_clearAction(nullptr)
    , m_filenameSearchWidget(nullptr)
    , m_contentSearchWidget(nullptr)
{
    setWindowTitle("DFM Search 示例");
    resize(900, 700);
    
    // 注册搜索引擎
    DFM::Search::registerFilenameSearchEngines();
    DFM::Search::registerContentSearchEngines();
    
    // 创建搜索引擎管理器
    m_searchManager = std::make_shared<SearchEngineManager>(this);
    
    // 添加搜索引擎
    m_searchManager->addEngine(SearchEngineFactory::createEngine(
        SearchType::FileName, SearchMode::Realtime, this));
    m_searchManager->addEngine(SearchEngineFactory::createEngine(
        SearchType::Fulltext, SearchMode::Realtime, this));
    
    connect(m_searchManager.get(), &SearchEngineManager::searchCompleted,
            this, &MainWindow::onSearchCompleted);
    connect(m_searchManager.get(), &SearchEngineManager::errorOccurred,
            this, &MainWindow::onSearchError);
    
    // 设置UI
    setupActions();
    setupMenus();
    setupToolbar();
    setupStatusBar();
    setupCentralWidget();
}

MainWindow::~MainWindow()
{
    // 清理
    if (m_searchManager) {
        m_searchManager->cancelAll();
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, "关于 DFM Search 示例",
        "<h3>DFM Search 示例</h3>"
        "<p>这是一个基于 DFM Search 库的示例应用，"
        "演示了如何使用文件名搜索和文件内容搜索功能。</p>"
        "<p>支持以下功能:</p>"
        "<ul>"
        "<li>文件名搜索 (实时)</li>"
        "<li>文件内容搜索 (实时)</li>"
        "<li>各种搜索选项和过滤器</li>"
        "</ul>"
        "<p>&copy; 2023 DFM 团队</p>");
}

void MainWindow::onSearchCompleted(SearchType type, bool success)
{
    QString searchTypeStr = (type == SearchType::FileName) ? "文件名搜索" : "内容搜索";
    QString resultStr = success ? "成功" : "失败";
    
    m_statusBar->showMessage(QString("%1 完成：%2").arg(searchTypeStr).arg(resultStr), 3000);
}

void MainWindow::onSearchError(SearchType type, const QString& error)
{
    QString searchTypeStr = (type == SearchType::FileName) ? "文件名搜索" : "内容搜索";
    
    m_statusBar->showMessage(QString("%1 错误：%2").arg(searchTypeStr).arg(error), 5000);
    QMessageBox::warning(this, "搜索错误", 
        QString("%1 发生错误：\n%2").arg(searchTypeStr).arg(error));
}

void MainWindow::setupMenus()
{
    m_fileMenu = menuBar()->addMenu("文件(&F)");
    m_fileMenu->addAction(m_exitAction);
    
    m_searchMenu = menuBar()->addMenu("搜索(&S)");
    m_searchMenu->addAction(m_clearAction);
    
    m_helpMenu = menuBar()->addMenu("帮助(&H)");
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupActions()
{
    m_exitAction = new QAction("退出(&Q)", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    m_aboutAction = new QAction("关于(&A)", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
    
    m_clearAction = new QAction("清除搜索(&C)", this);
    m_clearAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(m_clearAction, &QAction::triggered, [this]() {
        if (m_tabWidget->currentIndex() == 0 && m_filenameSearchWidget) {
            m_filenameSearchWidget->clearSearch();
        } else if (m_tabWidget->currentIndex() == 1 && m_contentSearchWidget) {
            m_contentSearchWidget->clearSearch();
        }
    });
}

void MainWindow::setupToolbar()
{
    m_toolbar = addToolBar("主工具栏");
    m_toolbar->setMovable(false);
    m_toolbar->addAction(m_clearAction);
    m_toolbar->addAction(m_aboutAction);
    m_toolbar->addSeparator();
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("就绪", 3000);
}

void MainWindow::setupCentralWidget()
{
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(centralWidget);
    
    // 创建文件名搜索页
    m_filenameSearchWidget = new SearchWidget(
        m_searchManager->createEngine(SearchType::FileName, SearchMode::Realtime),
        m_tabWidget);
    m_tabWidget->addTab(m_filenameSearchWidget, "文件名搜索");
    
    // 创建内容搜索页
    m_contentSearchWidget = new SearchWidget(
        m_searchManager->createEngine(SearchType::Fulltext, SearchMode::Realtime),
        m_tabWidget);
    m_tabWidget->addTab(m_contentSearchWidget, "内容搜索");
    
    // 设置默认搜索路径
    QString defaultPath = QDir::homePath();
    
    SearchScope scope;
    scope.includePaths << defaultPath;
    
    m_filenameSearchWidget->setScope(scope);
    m_contentSearchWidget->setScope(scope);
    
    // 添加标签页到主布局
    mainLayout->addWidget(m_tabWidget);
    
    setCentralWidget(centralWidget);
}
