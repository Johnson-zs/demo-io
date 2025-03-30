#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchwidget.h"

#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_searchWidget(new SearchWidget(this))
{
    ui->setupUi(this);
    
    // 添加搜索组件到主窗口
    QVBoxLayout *layout = new QVBoxLayout(ui->searchFrame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_searchWidget);
    ui->searchFrame->setLayout(layout);
    
    // 设置默认搜索类型和模式
    m_searchWidget->setSearchType(DFM::Search::SearchType::Filename);
    m_searchWidget->setSearchMode(DFM::Search::SearchMode::Realtime);
    
    // 连接切换信号
    connect(ui->searchTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSearchTypeChanged);
    connect(ui->searchModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSearchModeChanged);
    
    // 设置菜单动作
    setupMenus();
    
    // 设置窗口大小和标题
    resize(900, 650);
    setWindowTitle(tr("DFM Search Example"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSearchTypeChanged(int index)
{
    // 根据选择的索引设置搜索类型
    switch (index) {
        case 0:
            m_searchWidget->setSearchType(DFM::Search::SearchType::Filename);
            break;
        case 1:
            m_searchWidget->setSearchType(DFM::Search::SearchType::Fulltext);
            break;
        case 2:
            m_searchWidget->setSearchType(DFM::Search::SearchType::Application);
            break;
        default:
            m_searchWidget->setSearchType(DFM::Search::SearchType::Filename);
            break;
    }
}

void MainWindow::onSearchModeChanged(int index)
{
    // 根据选择的索引设置搜索模式
    switch (index) {
        case 0:
            m_searchWidget->setSearchMode(DFM::Search::SearchMode::Realtime);
            break;
        case 1:
            m_searchWidget->setSearchMode(DFM::Search::SearchMode::Indexed);
            break;
        default:
            m_searchWidget->setSearchMode(DFM::Search::SearchMode::Realtime);
            break;
    }
}

void MainWindow::setupSearchConnections()
{
    // 这里可以添加一些与搜索相关的连接
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("关于 DFM Search"),
                      tr("DFM Search 示例应用程序\n"
                         "版本：0.1.0\n\n"
                         "这是一个演示 dfm-search 库功能的示例应用。\n"
                         "支持实时和索引化的文件名搜索。"));
}

void MainWindow::setupMenus()
{
    // 连接菜单动作
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::setupActions()
{
    // 这里可以添加工具栏动作等
} 