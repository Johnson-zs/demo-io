#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QtConcurrent>
#include <QDesktopServices>
#include <QClipboard>
#include <QMenu>
#include <QDir>
#include <QUrl>

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
    
    // 连接搜索选项改变信号
    connect(m_caseSensitiveCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onSearchOptionChanged);
    connect(m_fuzzySearchCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onSearchOptionChanged);

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

    // 添加双击打开文件功能
    connect(m_fileListView, &QListView::doubleClicked, this, &MainWindow::openSelectedFile);
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

    // 创建搜索框布局，添加帮助按钮
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(5);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("输入关键词搜索文件... (支持type:doc和py:pinyin语法)");
    m_searchLineEdit->setClearButtonEnabled(true);
    
    // 创建搜索选项
    m_searchOptionsLayout = new QHBoxLayout();
    m_caseSensitiveCheckBox = new QCheckBox("区分大小写", this);
    m_fuzzySearchCheckBox = new QCheckBox("模糊搜索", this);
    
    // 添加工具提示
    m_caseSensitiveCheckBox->setToolTip("启用大小写敏感搜索");
    m_fuzzySearchCheckBox->setToolTip("启用模糊匹配，可以找到拼写相近的文件");
    
    m_searchOptionsLayout->addWidget(m_caseSensitiveCheckBox);
    m_searchOptionsLayout->addWidget(m_fuzzySearchCheckBox);
    m_searchOptionsLayout->addStretch(); // 添加弹性空间

    // 添加searchLayout代替直接添加m_searchLineEdit
    searchLayout->addWidget(m_searchLineEdit, 1);
    
    QPushButton *helpButton = new QPushButton("?", this);
    helpButton->setFixedSize(24, 24);
    helpButton->setToolTip("搜索帮助");
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::showSearchHelp);
    searchLayout->addWidget(helpButton);
    
    // 添加searchLayout到主布局
    mainLayout->addLayout(searchLayout);

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

    // 创建右键菜单
    m_contextMenu = new QMenu(this);
    m_openFolderAction = m_contextMenu->addAction("打开所在文件夹");
    m_openFileAction = m_contextMenu->addAction("打开文件");
    m_copyPathAction = m_contextMenu->addAction("复制文件路径");
    
    // 连接信号
    connect(m_openFolderAction, &QAction::triggered, this, &MainWindow::openContainingFolder);
    connect(m_openFileAction, &QAction::triggered, this, &MainWindow::openSelectedFile);
    connect(m_copyPathAction, &QAction::triggered, this, &MainWindow::copyFilePath);
    
    // 连接列表视图的自定义上下文菜单
    m_fileListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fileListView, &QListView::customContextMenuRequested, 
            this, &MainWindow::showContextMenu);

    // 添加组件到布局
    mainLayout->addWidget(m_pathButton);
    mainLayout->addLayout(m_searchOptionsLayout);
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
    
    // 添加进度提示，让用户知道将要开始搜索
    m_statusLabel->setText("准备搜索...");
}

void MainWindow::onSearchOptionChanged()
{
    // 如果有搜索文本，则触发新的搜索
    if (!m_searchLineEdit->text().isEmpty()) {
        m_searchDebounceTimer->start(); // 重新启动防抖定时器触发搜索
    }
}

void MainWindow::loadMoreResults()
{
    if (m_currentSearchKeyword.isEmpty() || m_isLoadingMore) {
        return;
    }

    // 设置加载状态，防止重复触发
    m_isLoadingMore = true;
    
    // 获取搜索选项
    bool caseSensitive = m_caseSensitiveCheckBox->isChecked();
    bool fuzzySearch = m_fuzzySearchCheckBox->isChecked();

    // 使用异步加载，避免阻塞UI
    QtConcurrent::run([this, caseSensitive, fuzzySearch]() {
        m_currentOffset += m_currentBatchSize;
        QVector<FileData> moreBatch = m_searchManager->searchFilesBatch(
                m_currentSearchKeyword, m_currentOffset, m_currentBatchSize, 
                caseSensitive, fuzzySearch);
        
        // 在主线程中更新UI
        QMetaObject::invokeMethod(this, [this, moreBatch]() {
            if (!moreBatch.isEmpty()) {
                // 追加到模型，而不是重置
                m_fileModel->appendFileList(moreBatch);
            }
            m_isLoadingMore = false;
        }, Qt::QueuedConnection);
    });
}

void MainWindow::performSearch()
{
    QString searchText = m_searchLineEdit->text();
    m_currentSearchKeyword = searchText;
    m_currentOffset = 0;

    // 获取搜索选项
    bool caseSensitive = m_caseSensitiveCheckBox->isChecked();
    bool fuzzySearch = m_fuzzySearchCheckBox->isChecked();

    if (searchText.isEmpty()) {
        m_fileModel->setFileList(QVector<FileData>());
        m_searchManager->clearSearchResults();
        return;   // 状态已通过clearSearchResults更新
    } else {
        // 使用异步搜索而不是直接获取结果
        m_searchManager->searchFilesAsync(searchText, caseSensitive, fuzzySearch);
        
        // 更新UI，显示正在搜索的状态
        m_statusLabel->setText("正在搜索...");
        
        // 连接异步搜索结果信号
        connect(m_searchManager, &SearchManager::searchResultsReady, this, &MainWindow::onSearchResultsReady, Qt::UniqueConnection);
    }
}

void MainWindow::onSearchResultsReady(const QVector<FileData> &results)
{
    // 更新模型
    m_fileModel->setFileList(results);
    
    // 将关键词传递给模型用于高亮
    m_fileModel->setHighlightKeyword(m_currentSearchKeyword);
    
    // 更新状态栏
    updateStatusBar(results.size());
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

void MainWindow::showContextMenu(const QPoint &pos)
{
    // 获取点击的项目索引
    QModelIndex index = m_fileListView->indexAt(pos);
    
    if (index.isValid()) {
        // 启用所有操作
        m_openFolderAction->setEnabled(true);
        m_openFileAction->setEnabled(true);
        m_copyPathAction->setEnabled(true);
        
        // 检查是否是目录
        bool isDirectory = m_fileModel->data(index, FileListModel::IsDirectoryRole).toBool();
        if (isDirectory) {
            m_openFileAction->setText("打开文件夹");
        } else {
            m_openFileAction->setText("打开文件");
        }
        
        // 显示上下文菜单
        m_contextMenu->exec(m_fileListView->viewport()->mapToGlobal(pos));
    }
}

void MainWindow::openContainingFolder()
{
    QModelIndex index = m_fileListView->currentIndex();
    if (index.isValid()) {
        QString filePath = m_fileModel->data(index, FileListModel::PathRole).toString();
        QFileInfo fileInfo(filePath);
        
        // 获取文件所在的目录
        QString directoryPath = fileInfo.dir().absolutePath();
        
        // 使用默认程序打开文件夹
        QUrl directoryUrl = QUrl::fromLocalFile(directoryPath);
        QDesktopServices::openUrl(directoryUrl);
    }
}

void MainWindow::openSelectedFile()
{
    QModelIndex index = m_fileListView->currentIndex();
    if (index.isValid()) {
        QString filePath = m_fileModel->data(index, FileListModel::PathRole).toString();
        
        // 使用默认程序打开文件
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        QDesktopServices::openUrl(fileUrl);
    }
}

void MainWindow::copyFilePath()
{
    QModelIndex index = m_fileListView->currentIndex();
    if (index.isValid()) {
        QString filePath = m_fileModel->data(index, FileListModel::PathRole).toString();
        
        // 复制路径到剪贴板
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(filePath);
        
        // 提示用户
        m_statusLabel->setText("已复制: " + filePath);
    }
}

// 添加搜索帮助方法
void MainWindow::showSearchHelp()
{
    QMessageBox::information(this, "搜索帮助", 
        "基本搜索:\n"
        "  直接输入关键词搜索文件名\n\n"
        "文件类型搜索:\n"
        "  type:doc,pdf - 搜索指定类型的文件\n"
        "  report type:xlsx - 搜索名称包含'report'的Excel文件\n\n"
        "拼音搜索:\n"
        "  py:zhang - 搜索拼音包含'zhang'的文件\n"
        "  py:zhang,li - 搜索拼音包含'zhang'或'li'的文件\n"
        "  文档 py:wendang - 搜索名称包含'文档'且拼音含'wendang'的文件\n\n"
        "搜索选项:\n"
        "  区分大小写 - 启用大小写敏感搜索\n"
        "  模糊搜索 - 查找拼写相近的文件");
}
