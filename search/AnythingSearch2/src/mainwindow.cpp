#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentPath(QDir::homePath()), m_currentBatchSize(100), m_currentOffset(0)
{
    setupUI();

    // 创建防抖定时器
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setSingleShot(true);
    m_searchDebounceTimer->setInterval(300);   // 300ms防抖

    connect(m_searchDebounceTimer, &QTimer::timeout, this, &MainWindow::performSearch);
    connect(m_pathButton, &QPushButton::clicked, this, &MainWindow::selectSearchPath);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    // 设置路径按钮文本
    m_pathButton->setText(QString("搜索路径: %1").arg(m_currentPath));

    // 创建搜索管理器
    m_searchManager = new SearchManager(this);

    // 初始索引
    m_searchManager->updateSearchPath(m_currentPath);

    // 连接滚动条到加载更多方法
    m_scrollBar = m_fileListView->verticalScrollBar();
    connect(m_scrollBar, &QScrollBar::valueChanged, this, [this](int value) {
        if (value >= m_scrollBar->maximum() * 0.8) {
            loadMoreResults();
        }
    });

    connect(m_searchManager, &SearchManager::searchStatusChanged,
            this, &MainWindow::onSearchStatusChanged);
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

    // 设置状态栏
    setupStatusBar();
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignRight);
    m_statusBar->addPermanentWidget(m_statusLabel, 1);
    updateStatusBar(0);
}

void MainWindow::updateStatusBar(int count)
{
    m_statusLabel->setText(QString("找到 %1 个文件").arg(count));
}

void MainWindow::selectSearchPath()
{
    QString path = QFileDialog::getExistingDirectory(
            this, "选择搜索路径", m_currentPath,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()) {
        m_currentPath = path;
        m_pathButton->setText(QString("搜索路径: %1").arg(path));
        m_searchManager->updateSearchPath(path);
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    // 如果输入为空，清空搜索结果并取消防抖
    if (text.isEmpty()) {
        m_searchDebounceTimer->stop();
        m_fileModel->setFileList(QVector<FileData>());
        m_searchManager->clearSearchResults();
        updateStatusBar(0);
        return;
    }

    // 取消之前的搜索
    m_searchManager->cancelSearch();

    // 重置防抖定时器
    m_searchDebounceTimer->start();
}

void MainWindow::loadMoreResults()
{
    if (m_currentSearchKeyword.isEmpty()) {
        return;
    }

    m_currentOffset += m_currentBatchSize;
    QVector<FileData> moreBatch = m_searchManager->searchFilesBatch(
            m_currentSearchKeyword, m_currentOffset, m_currentBatchSize);

    if (!moreBatch.isEmpty()) {
        // 追加到模型，而不是重置
        m_fileModel->appendFileList(moreBatch);
    }
}

void MainWindow::performSearch()
{
    QString searchText = m_searchLineEdit->text();
    m_currentSearchKeyword = searchText;
    m_currentOffset = 0;

    if (searchText.isEmpty()) {
        m_fileModel->setFileList(QVector<FileData>());
        m_searchManager->clearSearchResults();
        return;   // 状态已通过clearSearchResults更新
    } else {
        // 获取第一批结果
        QVector<FileData> firstBatch = m_searchManager->searchFilesBatch(
                searchText, 0, m_currentBatchSize);
        m_fileModel->setFileList(firstBatch);

        // 将关键词传递给模型用于高亮
        m_fileModel->setHighlightKeyword(searchText);
    }
}

void MainWindow::onSearchStatusChanged(SearchManager::SearchStatus status, const QString &message)
{
    switch (status) {
    case SearchManager::Searching:
        m_statusLabel->setText(message);
        QApplication::setOverrideCursor(Qt::WaitCursor);   // 显示等待光标
        break;

    case SearchManager::Completed:
        m_statusLabel->setText(message);
        QApplication::restoreOverrideCursor();   // 恢复正常光标
        break;

    case SearchManager::Error:
        m_statusLabel->setText("搜索出错: " + message);
        QApplication::restoreOverrideCursor();
        break;

    case SearchManager::Idle:
    default:
        if (!message.isEmpty()) {
            m_statusLabel->setText(message);
        } else {
            m_statusLabel->setText("就绪");
        }
        QApplication::restoreOverrideCursor();
        break;
    }
}
