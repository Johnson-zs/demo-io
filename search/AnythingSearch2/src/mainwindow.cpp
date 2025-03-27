#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentPath(QDir::homePath())
{
    setupUI();
    
    // 创建防抖定时器
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setSingleShot(true);
    m_searchDebounceTimer->setInterval(300); // 300ms防抖
    
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &MainWindow::performSearch);
    connect(m_pathButton, &QPushButton::clicked, this, &MainWindow::selectSearchPath);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // 设置路径按钮文本
    m_pathButton->setText(QString("搜索路径: %1").arg(m_currentPath));
    
    // 创建搜索管理器
    m_searchManager = new SearchManager(this);
    
    // 初始索引
    m_searchManager->updateSearchPath(m_currentPath);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央窗口和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 创建路径选择按钮
    m_pathButton = new QPushButton(this);
    m_pathButton->setIcon(QIcon(":/icons/folder.png"));
    
    // 创建搜索框
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("输入关键词搜索文件...");
    m_searchLineEdit->setClearButtonEnabled(true);
    
    // 创建文件列表视图
    m_fileListView = new QListView(this);
    m_fileListView->setUniformItemSizes(true);
    m_fileListView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 创建和设置模型
    m_fileModel = new FileListModel(this);
    m_fileListView->setModel(m_fileModel);
    
    // 创建和设置代理
    m_fileDelegate = new FileItemDelegate(this);
    m_fileListView->setItemDelegate(m_fileDelegate);
    
    // 添加组件到布局
    mainLayout->addWidget(m_pathButton);
    mainLayout->addWidget(m_searchLineEdit);
    mainLayout->addWidget(m_fileListView, 1);
    
    setCentralWidget(centralWidget);
    resize(800, 600);
    setWindowTitle("文件搜索工具");
}

void MainWindow::selectSearchPath()
{
    QString path = QFileDialog::getExistingDirectory(
        this, "选择搜索路径", m_currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!path.isEmpty()) {
        m_currentPath = path;
        m_pathButton->setText(QString("搜索路径: %1").arg(path));
        m_searchManager->updateSearchPath(path);
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    // 重置防抖定时器
    m_searchDebounceTimer->start();
}

void MainWindow::performSearch()
{
    QString searchText = m_searchLineEdit->text();
    
    if (searchText.isEmpty()) {
        // 显示全部文件，限制数量以保证性能
        m_fileModel->setFileList(m_searchManager->getAllFiles(1000));
    } else {
        // 执行搜索
        m_fileModel->setFileList(m_searchManager->searchFiles(searchText));
    }
} 