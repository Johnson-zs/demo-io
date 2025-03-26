#include "advanced_search_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QStandardItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QUrl>

AdvancedSearchWindow::AdvancedSearchWindow(QWidget* parent)
    : QMainWindow(parent), 
      currentSearchId(-1),
      useWorkerMode(false)
{
    setupUI();
    setupConnections();
    
    // 初始化搜索管理器
    indexManager = &QSearch::IndexManager::instance();
    
    // 初始状态
    searchButton->setEnabled(true);
    clearButton->setEnabled(false);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    statusLabel->setText("就绪");
}

AdvancedSearchWindow::~AdvancedSearchWindow() {
}

void AdvancedSearchWindow::setUseWorker(bool useWorker) {
    useWorkerMode = useWorker;
    if (useWorker) {
        statusLabel->setText("启用工作进程模式");
    } else {
        statusLabel->setText("使用本地搜索模式");
    }
}

void AdvancedSearchWindow::setIndexPath(const QString& path) {
    indexPath = path;
    indexManager->setBasePath(path);
    updateIndexStatus();
}

void AdvancedSearchWindow::setupUI() {
    // 创建中央小部件和主布局
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建标签页小部件
    mainTabWidget = new QTabWidget(this);
    
    // 搜索标签页
    QWidget* searchTab = new QWidget(this);
    QVBoxLayout* searchLayout = new QVBoxLayout(searchTab);
    
    // 搜索栏
    QHBoxLayout* searchBarLayout = new QHBoxLayout();
    searchTextEdit = new QLineEdit(this);
    searchTextEdit->setPlaceholderText("输入搜索关键词...");
    searchTypeCombo = new QComboBox(this);
    searchTypeCombo->addItem("文件名", QSearch::QueryType::Filename);
    searchTypeCombo->addItem("文件内容", QSearch::QueryType::FileContent);
    searchTypeCombo->addItem("两者", QSearch::QueryType::Both);
    
    matchTypeCombo = new QComboBox(this);
    matchTypeCombo->addItem("包含", QSearch::MatchType::Contains);
    matchTypeCombo->addItem("完全匹配", QSearch::MatchType::Exact);
    matchTypeCombo->addItem("开头匹配", QSearch::MatchType::StartsWith);
    matchTypeCombo->addItem("结尾匹配", QSearch::MatchType::EndsWith);
    matchTypeCombo->addItem("正则表达式", QSearch::MatchType::Regex);
    
    searchButton = new QPushButton("搜索", this);
    clearButton = new QPushButton("清除", this);
    
    searchBarLayout->addWidget(searchTextEdit);
    searchBarLayout->addWidget(searchTypeCombo);
    searchBarLayout->addWidget(matchTypeCombo);
    searchBarLayout->addWidget(searchButton);
    searchBarLayout->addWidget(clearButton);
    
    // 高级选项分组
    QGroupBox* advancedGroup = new QGroupBox("高级选项", this);
    QGridLayout* advancedLayout = new QGridLayout(advancedGroup);
    
    // 路径选择
    advancedLayout->addWidget(new QLabel("搜索路径:"), 0, 0);
    pathEdit = new QLineEdit(QDir::homePath(), this);
    advancedLayout->addWidget(pathEdit, 0, 1, 1, 2);
    browseButton = new QPushButton("浏览...", this);
    advancedLayout->addWidget(browseButton, 0, 3);
    
    // 复选框
    recursiveCheckBox = new QCheckBox("递归子目录", this);
    recursiveCheckBox->setChecked(true);
    hiddenFilesCheckBox = new QCheckBox("包含隐藏文件", this);
    caseSensitiveCheckBox = new QCheckBox("区分大小写", this);
    
    advancedLayout->addWidget(recursiveCheckBox, 1, 0);
    advancedLayout->addWidget(hiddenFilesCheckBox, 1, 1);
    advancedLayout->addWidget(caseSensitiveCheckBox, 1, 2);
    
    // 日期范围
    advancedLayout->addWidget(new QLabel("修改日期:"), 2, 0);
    fromDateEdit = new QDateEdit(this);
    fromDateEdit->setDate(QDate::currentDate().addMonths(-1));
    fromDateEdit->setCalendarPopup(true);
    advancedLayout->addWidget(fromDateEdit, 2, 1);
    
    advancedLayout->addWidget(new QLabel("至"), 2, 2);
    toDateEdit = new QDateEdit(this);
    toDateEdit->setDate(QDate::currentDate());
    toDateEdit->setCalendarPopup(true);
    advancedLayout->addWidget(toDateEdit, 2, 3);
    
    // 文件大小范围
    advancedLayout->addWidget(new QLabel("文件大小:"), 3, 0);
    minSizeSpinBox = new QSpinBox(this);
    minSizeSpinBox->setRange(0, 99999);
    minSizeSpinBox->setValue(0);
    advancedLayout->addWidget(minSizeSpinBox, 3, 1);
    
    advancedLayout->addWidget(new QLabel("至"), 3, 2);
    maxSizeSpinBox = new QSpinBox(this);
    maxSizeSpinBox->setRange(0, 99999);
    maxSizeSpinBox->setValue(0);
    maxSizeSpinBox->setSpecialValueText("不限");
    advancedLayout->addWidget(maxSizeSpinBox, 3, 3);
    
    sizeUnitCombo = new QComboBox(this);
    sizeUnitCombo->addItem("KB", 1024);
    sizeUnitCombo->addItem("MB", 1024*1024);
    sizeUnitCombo->addItem("GB", 1024*1024*1024);
    advancedLayout->addWidget(sizeUnitCombo, 3, 4);
    
    // 文件类型
    advancedLayout->addWidget(new QLabel("文件扩展名:"), 4, 0);
    extensionsEdit = new QLineEdit(this);
    extensionsEdit->setPlaceholderText("用逗号分隔, 例如: pdf,doc,txt");
    advancedLayout->addWidget(extensionsEdit, 4, 1, 1, 4);
    
    // 搜索设置
    advancedLayout->addWidget(new QLabel("最大结果数:"), 5, 0);
    maxResultsSpinBox = new QSpinBox(this);
    maxResultsSpinBox->setRange(10, 10000);
    maxResultsSpinBox->setValue(1000);
    maxResultsSpinBox->setSingleStep(100);
    advancedLayout->addWidget(maxResultsSpinBox, 5, 1);
    
    advancedLayout->addWidget(new QLabel("超时(秒):"), 5, 2);
    timeoutSpinBox = new QSpinBox(this);
    timeoutSpinBox->setRange(0, 300);
    timeoutSpinBox->setValue(30);
    timeoutSpinBox->setSpecialValueText("不限");
    advancedLayout->addWidget(timeoutSpinBox, 5, 3);
    
    // 排序选项
    advancedLayout->addWidget(new QLabel("排序方式:"), 6, 0);
    sortOrderCombo = new QComboBox(this);
    sortOrderCombo->addItem("默认", QSearch::SortOrder::NoSort);
    sortOrderCombo->addItem("名称 (升序)", QSearch::SortOrder::NameAsc);
    sortOrderCombo->addItem("名称 (降序)", QSearch::SortOrder::NameDesc);
    sortOrderCombo->addItem("大小 (升序)", QSearch::SortOrder::SizeAsc);
    sortOrderCombo->addItem("大小 (降序)", QSearch::SortOrder::SizeDesc);
    sortOrderCombo->addItem("日期 (升序)", QSearch::SortOrder::DateAsc);
    sortOrderCombo->addItem("日期 (降序)", QSearch::SortOrder::DateDesc);
    sortOrderCombo->addItem("相关性", QSearch::SortOrder::RelevanceDesc);
    advancedLayout->addWidget(sortOrderCombo, 6, 1, 1, 2);
    
    // 结果视图
    resultsTreeView = new QTreeView(this);
    resultsTreeView->setSortingEnabled(true);
    resultsTreeView->setRootIsDecorated(false);
    resultsTreeView->setAlternatingRowColors(true);
    resultsTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    resultsModel = new QStandardItemModel(this);
    resultsModel->setColumnCount(5);
    resultsModel->setHorizontalHeaderLabels(
        QStringList() << "名称" << "路径" << "大小" << "修改日期" << "类型");
    
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(resultsModel);
    resultsTreeView->setModel(proxyModel);
    
    // 状态栏
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLabel = new QLabel("就绪", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setVisible(false);
    
    statusLayout->addWidget(statusLabel, 1);
    statusLayout->addWidget(progressBar);
    
    // 添加所有元素到搜索标签页
    searchLayout->addLayout(searchBarLayout);
    searchLayout->addWidget(advancedGroup);
    searchLayout->addWidget(resultsTreeView, 1);
    searchLayout->addLayout(statusLayout);
    
    // 索引标签页
    QWidget* indexTab = new QWidget(this);
    QVBoxLayout* indexLayout = new QVBoxLayout(indexTab);
    
    QGroupBox* indexGroup = new QGroupBox("索引管理", this);
    QGridLayout* indexGridLayout = new QGridLayout(indexGroup);
    
    indexGridLayout->addWidget(new QLabel("索引类型:"), 0, 0);
    indexTypeCombo = new QComboBox(this);
    indexTypeCombo->addItem("文件名索引", static_cast<int>(QSearch::IndexType::Filename));
    indexTypeCombo->addItem("文件内容索引", static_cast<int>(QSearch::IndexType::FileContent));
    indexGridLayout->addWidget(indexTypeCombo, 0, 1, 1, 2);
    
    createIndexButton = new QPushButton("创建索引", this);
    updateIndexButton = new QPushButton("更新索引", this);
    indexGridLayout->addWidget(createIndexButton, 1, 0);
    indexGridLayout->addWidget(updateIndexButton, 1, 1);
    
    indexStatusLabel = new QLabel("索引状态: 未初始化", this);
    indexProgressBar = new QProgressBar(this);
    indexProgressBar->setRange(0, 100);
    indexProgressBar->setVisible(false);
    
    indexGridLayout->addWidget(indexStatusLabel, 2, 0, 1, 3);
    indexGridLayout->addWidget(indexProgressBar, 3, 0, 1, 3);
    
    indexLayout->addWidget(indexGroup);
    indexLayout->addStretch(1);
    
    // 添加标签页到主标签页控件
    mainTabWidget->addTab(searchTab, "搜索");
    mainTabWidget->addTab(indexTab, "索引管理");
    
    // 将标签页控件添加到主布局
    mainLayout->addWidget(mainTabWidget);
    
    // 设置中央小部件
    setCentralWidget(centralWidget);
    
    // 调整窗口大小
    resize(900, 700);
    setWindowTitle("QSearch 高级搜索示例");
}

void AdvancedSearchWindow::setupConnections() {
    // 搜索相关连接
    connect(searchButton, &QPushButton::clicked, this, &AdvancedSearchWindow::onSearch);
    connect(clearButton, &QPushButton::clicked, this, &AdvancedSearchWindow::onClear);
    connect(browseButton, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(
            this, "选择搜索目录", pathEdit->text(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            pathEdit->setText(dir);
        }
    });
    
    // 索引相关连接
    connect(indexTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchWindow::onIndexSelected);
    connect(createIndexButton, &QPushButton::clicked, this, &AdvancedSearchWindow::onCreateIndex);
    connect(updateIndexButton, &QPushButton::clicked, this, &AdvancedSearchWindow::onUpdateIndex);
    
    // 标签页切换
    connect(mainTabWidget, &QTabWidget::currentChanged, this, &AdvancedSearchWindow::onTabChanged);
    
    // 搜索管理器信号
    connect(&searchManager, &QSearch::SearchManager::searchProgressChanged,
            this, &AdvancedSearchWindow::onSearchProgressChanged);
    connect(&searchManager, &QSearch::SearchManager::resultItemFound,
            this, &AdvancedSearchWindow::onResultItemFound);
    connect(&searchManager, &QSearch::SearchManager::searchCompleted,
            this, &AdvancedSearchWindow::onSearchCompleted);
    connect(&searchManager, &QSearch::SearchManager::searchError,
            this, &AdvancedSearchWindow::onSearchError);
    
    // 结果上下文菜单
    connect(resultsTreeView, &QTreeView::customContextMenuRequested, [this](const QPoint& pos) {
        QModelIndex index = resultsTreeView->indexAt(pos);
        if (index.isValid()) {
            QMenu contextMenu(this);
            
            QAction* openAction = contextMenu.addAction("打开");
            QAction* openDirAction = contextMenu.addAction("打开所在文件夹");
            contextMenu.addSeparator();
            QAction* copyPathAction = contextMenu.addAction("复制路径");
            
            QAction* selectedAction = contextMenu.exec(resultsTreeView->viewport()->mapToGlobal(pos));
            
            if (selectedAction) {
                // 获取文件路径
                QModelIndex sourceIndex = proxyModel->mapToSource(index);
                QString filePath = resultsModel->data(
                    resultsModel->index(sourceIndex.row(), 1)).toString();
                
                if (selectedAction == openAction) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
                } else if (selectedAction == openDirAction) {
                    QFileInfo info(filePath);
                    QDesktopServices::openUrl(QUrl::fromLocalFile(info.dir().path()));
                } else if (selectedAction == copyPathAction) {
                    QGuiApplication::clipboard()->setText(filePath);
                }
            }
        }
    });
}

QSearch::SearchQuery AdvancedSearchWindow::buildQuery() {
    QSearch::SearchQuery query;
    
    // 基本查询参数
    query.setText(searchTextEdit->text())
         .setType(static_cast<QSearch::QueryType>(searchTypeCombo->currentData().toInt()))
         .setMatchType(static_cast<QSearch::MatchType>(matchTypeCombo->currentData().toInt()));
    
    // 搜索路径
    query.setPaths(QStringList() << pathEdit->text());
    
    // 文件过滤器
    if (!extensionsEdit->text().isEmpty()) {
        QStringList extensions = extensionsEdit->text().split(",", Qt::SkipEmptyParts);
        QStringList filters;
        for (const QString& ext : extensions) {
            filters << "*." + ext.trimmed();
        }
        query.setFileFilters(filters);
    }
    
    // 大小限制
    if (maxSizeSpinBox->value() > 0) {
        qint64 minSize = minSizeSpinBox->value() * sizeUnitCombo->currentData().toLongLong();
        qint64 maxSize = maxSizeSpinBox->value() * sizeUnitCombo->currentData().toLongLong();
        query.setSizeLimit(minSize, maxSize);
    }
    
    // 日期限制
    QDateTime fromDate = QDateTime(fromDateEdit->date(), QTime(0, 0, 0));
    QDateTime toDate = QDateTime(toDateEdit->date(), QTime(23, 59, 59));
    query.setTimeLimit(fromDate, toDate);
    
    // 额外选项
    query.setOption("include_hidden", hiddenFilesCheckBox->isChecked());
    query.setOption("recursive", recursiveCheckBox->isChecked());
    
    return query;
}

QSearch::SearchOptions AdvancedSearchWindow::buildOptions() {
    QSearch::SearchOptions options;
    
    // 基本选项
    options.setMaxResults(maxResultsSpinBox->value())
           .setCaseSensitive(caseSensitiveCheckBox->isChecked())
           .setSortOrder(static_cast<QSearch::SortOrder>(sortOrderCombo->currentData().toInt()));
    
    // 设置超时
    if (timeoutSpinBox->value() > 0) {
        options.setTimeoutMs(timeoutSpinBox->value() * 1000);
    }
    
    // 使用索引的设置
    QSearch::IndexType indexType = static_cast<QSearch::IndexType>(
        searchTypeCombo->currentData().toInt() == QSearch::QueryType::FileContent ?
        QSearch::IndexType::FileContent : QSearch::IndexType::Filename);
    
    if (indexManager->isIndexReady(indexType)) {
        options.setSearchMode(QSearch::SearchMode::Indexed);
    } else {
        options.setSearchMode(QSearch::SearchMode::Normal);
    }
    
    // Worker模式设置
    options.setOption("use_worker", useWorkerMode);
    
    return options;
}

void AdvancedSearchWindow::onSearch() {
    // 清除旧结果
    resultsModel->removeRows(0, resultsModel->rowCount());
    
    // 构建查询和选项
    QSearch::SearchQuery query = buildQuery();
    QSearch::SearchOptions options = buildOptions();
    
    // 开始搜索
    currentSearchId = searchManager.startSearch(query, options);
    
    if (currentSearchId < 0) {
        showStatusMessage("搜索启动失败", true);
        return;
    }
    
    // 更新UI状态
    searchButton->setEnabled(false);
    clearButton->setEnabled(true);
    progressBar->setValue(0);
    progressBar->setVisible(true);
    showStatusMessage("搜索中...");
}

void AdvancedSearchWindow::onClear() {
    // 停止当前搜索
    if (currentSearchId >= 0) {
        searchManager.stopSearch(currentSearchId);
    }
    
    // 清除结果
    resultsModel->removeRows(0, resultsModel->rowCount());
    
    // 重置UI状态
    searchButton->setEnabled(true);
    clearButton->setEnabled(false);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    showStatusMessage("就绪");
}

void AdvancedSearchWindow::onIndexSelected(int index) {
    updateIndexStatus();
}

void AdvancedSearchWindow::onTabChanged(int index) {
    if (index == 1) {  // 索引标签页
        updateIndexStatus();
    }
}

void AdvancedSearchWindow::onCreateIndex() {
    QSearch::IndexType indexType = static_cast<QSearch::IndexType>(
        indexTypeCombo->currentData().toInt());
    
    // 获取要索引的路径
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择要索引的目录", QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (dir.isEmpty()) {
        return;
    }
    
    // 开始创建索引
    if (indexManager->createIndex(indexType, QStringList() << dir)) {
        indexStatusLabel->setText("正在创建索引...");
        indexProgressBar->setValue(0);
        indexProgressBar->setVisible(true);
        createIndexButton->setEnabled(false);
        updateIndexButton->setEnabled(false);
        
        // 连接索引进度信号
        connect(indexManager, &QSearch::IndexManager::indexProgressChanged,
            [this](QSearch::IndexType type, double progress) {
                QSearch::IndexType currentType = static_cast<QSearch::IndexType>(
                    indexTypeCombo->currentData().toInt());
                
                if (type == currentType) {
                    indexProgressBar->setValue(static_cast<int>(progress * 100));
                }
            });
        
        // 连接索引完成信号
        connect(indexManager, &QSearch::IndexManager::indexCompleted,
            [this](QSearch::IndexType type) {
                QSearch::IndexType currentType = static_cast<QSearch::IndexType>(
                    indexTypeCombo->currentData().toInt());
                
                if (type == currentType) {
                    updateIndexStatus();
                    indexProgressBar->setVisible(false);
                    createIndexButton->setEnabled(true);
                    updateIndexButton->setEnabled(true);
                    
                    QMessageBox::information(this, "索引完成", 
                        "索引创建完成，现在可以使用索引加速搜索。");
                }
            });
    } else {
        QMessageBox::warning(this, "索引错误", "无法启动索引创建。");
    }
}

void AdvancedSearchWindow::onUpdateIndex() {
    QSearch::IndexType indexType = static_cast<QSearch::IndexType>(
        indexTypeCombo->currentData().toInt());
    
    if (indexManager->updateIndex(indexType)) {
        indexStatusLabel->setText("正在更新索引...");
        indexProgressBar->setValue(0);
        indexProgressBar->setVisible(true);
        createIndexButton->setEnabled(false);
        updateIndexButton->setEnabled(false);
    } else {
        QMessageBox::warning(this, "索引错误", "无法启动索引更新。");
    }
}

void AdvancedSearchWindow::onSearchProgressChanged(int searchId, double progress) {
    if (searchId == currentSearchId) {
        progressBar->setValue(static_cast<int>(progress * 100));
    }
}

void AdvancedSearchWindow::onResultItemFound(int searchId, const QSearch::ResultItem& item) {
    if (searchId == currentSearchId) {
        displayResult(item);
    }
}

void AdvancedSearchWindow::onSearchCompleted(int searchId) {
    if (searchId == currentSearchId) {
        progressBar->setValue(100);
        showStatusMessage(QString("搜索完成，找到 %1 个结果").arg(resultsModel->rowCount()));
        
        searchButton->setEnabled(true);
        clearButton->setEnabled(true);
    }
}

void AdvancedSearchWindow::onSearchError(int searchId, const QString& error) {
    if (searchId == currentSearchId) {
        progressBar->setVisible(false);
        showStatusMessage(QString("搜索错误: %1").arg(error), true);
        
        searchButton->setEnabled(true);
        clearButton->setEnabled(true);
    }
}

void AdvancedSearchWindow::displayResult(const QSearch::ResultItem& item) {
    QList<QStandardItem*> rowItems;
    
    rowItems << new QStandardItem(item.name);
    rowItems << new QStandardItem(item.path);
    
    QString sizeStr;
    if (item.size < 1024) {
        sizeStr = QString("%1 B").arg(item.size);
    } else if (item.size < 1024*1024) {
        sizeStr = QString("%1 KB").arg(item.size / 1024.0, 0, 'f', 1);
    } else if (item.size < 1024*1024*1024) {
        sizeStr = QString("%1 MB").arg(item.size / (1024.0*1024.0), 0, 'f', 1);
    } else {
        sizeStr = QString("%1 GB").arg(item.size / (1024.0*1024.0*1024.0), 0, 'f', 1);
    }
    rowItems << new QStandardItem(sizeStr);
    
    rowItems << new QStandardItem(item.modifiedTime.toString("yyyy-MM-dd hh:mm:ss"));
    
    QFileInfo info(item.path);
    rowItems << new QStandardItem(info.suffix().toUpper());
    
    // 为目录项设置图标
    if (item.isDir) {
        rowItems[0]->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    } else {
        rowItems[0]->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    }
    
    resultsModel->appendRow(rowItems);
    
    // 自动调整列宽
    if (resultsModel->rowCount() == 1) {
        resultsTreeView->header()->resizeSections(QHeaderView::ResizeToContents);
    }
}

void AdvancedSearchWindow::updateIndexStatus() {
    QSearch::IndexType indexType = static_cast<QSearch::IndexType>(
        indexTypeCombo->currentData().toInt());
    
    // 检查索引状态
    QSearch::IndexStatus status = indexManager->indexStatus(indexType);
    QString statusStr;
    
    switch (status) {
        case QSearch::IndexStatus::NotInitialized:
            statusStr = "未初始化";
            break;
        case QSearch::IndexStatus::Initializing:
            statusStr = "正在初始化";
            break;
        case QSearch::IndexStatus::Ready:
            statusStr = "就绪";
            break;
        case QSearch::IndexStatus::Updating:
            statusStr = "正在更新";
            break;
        case QSearch::IndexStatus::Error:
            statusStr = "错误: " + indexManager->indexStatusMessage(indexType);
            break;
        default:
            statusStr = "未知";
    }
    
    // 获取索引统计信息
    qint64 fileCount = indexManager->indexedFileCount(indexType);
    qint64 indexSize = indexManager->indexSize(indexType);
    QDateTime lastUpdate = indexManager->lastUpdateTime(indexType);
    
    QString sizeStr;
    if (indexSize < 1024) {
        sizeStr = QString("%1 B").arg(indexSize);
    } else if (indexSize < 1024*1024) {
        sizeStr = QString("%1 KB").arg(indexSize / 1024.0, 0, 'f', 1);
    } else if (indexSize < 1024*1024*1024) {
        sizeStr = QString("%1 MB").arg(indexSize / (1024.0*1024.0), 0, 'f', 1);
    } else {
        sizeStr = QString("%1 GB").arg(indexSize / (1024.0*1024.0*1024.0), 0, 'f', 1);
    }
    
    QString lastUpdateStr = lastUpdate.isValid() 
        ? lastUpdate.toString("yyyy-MM-dd hh:mm:ss")
        : "从未更新";
    
    indexStatusLabel->setText(QString("状态: %1 | 文件数: %2 | 索引大小: %3 | 最后更新: %4")
        .arg(statusStr)
        .arg(fileCount)
        .arg(sizeStr)
        .arg(lastUpdateStr));
    
    // 更新按钮状态
    createIndexButton->setEnabled(status != QSearch::IndexStatus::Initializing && 
                                 status != QSearch::IndexStatus::Updating);
    updateIndexButton->setEnabled(status == QSearch::IndexStatus::Ready);
    
    // 更新进度条状态
    indexProgressBar->setVisible(status == QSearch::IndexStatus::Initializing || 
                                status == QSearch::IndexStatus::Updating);
}

void AdvancedSearchWindow::showStatusMessage(const QString& message, bool isError) {
    statusLabel->setText(message);
    
    if (isError) {
        statusLabel->setStyleSheet("color: red");
    } else {
        statusLabel->setStyleSheet("");
    }
} 