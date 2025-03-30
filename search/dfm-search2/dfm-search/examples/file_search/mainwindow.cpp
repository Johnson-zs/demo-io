#include "mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QTreeView>
#include <QStatusBar>
#include <QTimer>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , searchEngine(nullptr)
    , searchType("filename")
    , searchPath(QDir::homePath())
    , useIndex(false)
{
    setupUi();
    
    // 更新状态栏
    updateStatusBar("就绪");
    
    // 更新UI中的路径
    pathLineEdit->setText(searchPath);
    
    // 更新搜索选项
    updateSearchOptions();
}

void MainWindow::setupUi()
{
    // 创建中央部件和主布局
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建搜索组
    QGroupBox* searchGroupBox = new QGroupBox("搜索", centralWidget);
    QGridLayout* searchLayout = new QGridLayout(searchGroupBox);
    
    // 搜索类型标签
    searchTypeLabel = new QLabel("文件名搜索", searchGroupBox);
    searchLayout->addWidget(searchTypeLabel, 0, 0);
    
    // 搜索输入框
    searchLineEdit = new QLineEdit(searchGroupBox);
    searchLineEdit->setPlaceholderText("输入搜索关键词");
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &MainWindow::startSearch);
    searchLayout->addWidget(searchLineEdit, 0, 1, 1, 3);
    
    // 搜索按钮
    searchButton = new QPushButton("搜索", searchGroupBox);
    searchButton->setIcon(QIcon::fromTheme("search"));
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::startSearch);
    searchLayout->addWidget(searchButton, 0, 4);
    
    // 停止按钮
    stopButton = new QPushButton("停止", searchGroupBox);
    stopButton->setIcon(QIcon::fromTheme("process-stop"));
    stopButton->setEnabled(false);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopSearch);
    searchLayout->addWidget(stopButton, 0, 5);
    
    // 暂停按钮
    pauseButton = new QPushButton("暂停", searchGroupBox);
    pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    pauseButton->setEnabled(false);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::togglePause);
    searchLayout->addWidget(pauseButton, 0, 6);
    
    // 路径标签
    QLabel* pathLabel = new QLabel("搜索路径:", searchGroupBox);
    searchLayout->addWidget(pathLabel, 1, 0);
    
    // 路径输入框
    pathLineEdit = new QLineEdit(searchGroupBox);
    pathLineEdit->setReadOnly(true);
    searchLayout->addWidget(pathLineEdit, 1, 1, 1, 5);
    
    // 路径选择按钮
    pathButton = new QPushButton("浏览...", searchGroupBox);
    pathButton->setIcon(QIcon::fromTheme("folder-open"));
    connect(pathButton, &QPushButton::clicked, this, &MainWindow::selectSearchPath);
    searchLayout->addWidget(pathButton, 1, 6);
    
    // 添加搜索组到主布局
    mainLayout->addWidget(searchGroupBox);
    
    // 创建选项组
    QGroupBox* optionsGroupBox = new QGroupBox("搜索选项", centralWidget);
    QHBoxLayout* optionsLayout = new QHBoxLayout(optionsGroupBox);
    
    // 大小写敏感
    caseSensitiveCheckBox = new QCheckBox("区分大小写", optionsGroupBox);
    connect(caseSensitiveCheckBox, &QCheckBox::toggled, this, &MainWindow::updateSearchOptions);
    optionsLayout->addWidget(caseSensitiveCheckBox);
    
    // 正则表达式
    regexCheckBox = new QCheckBox("使用正则表达式", optionsGroupBox);
    connect(regexCheckBox, &QCheckBox::toggled, this, &MainWindow::updateSearchOptions);
    optionsLayout->addWidget(regexCheckBox);
    
    // 最大结果数
    QLabel* maxResultsLabel = new QLabel("最大结果数:", optionsGroupBox);
    optionsLayout->addWidget(maxResultsLabel);
    
    maxResultsSpinBox = new QSpinBox(optionsGroupBox);
    maxResultsSpinBox->setRange(1, 10000);
    maxResultsSpinBox->setValue(100);
    connect(maxResultsSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateSearchOptions);
    optionsLayout->addWidget(maxResultsSpinBox);
    
    // 使用索引
    indexCheckBox = new QCheckBox("使用索引", optionsGroupBox);
    indexCheckBox->setChecked(useIndex);
    connect(indexCheckBox, &QCheckBox::toggled, this, [this](bool checked) { useIndex = checked; });
    optionsLayout->addWidget(indexCheckBox);
    
    // 添加选项组到主布局
    mainLayout->addWidget(optionsGroupBox);
    
    // 创建结果视图
    resultsTreeView = new QTreeView(centralWidget);
    resultsTreeView->setAlternatingRowColors(true);
    resultsTreeView->setSortingEnabled(true);
    resultsTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(resultsTreeView, &QTreeView::doubleClicked, this, &MainWindow::handleResultDoubleClicked);
    
    // 初始化结果模型
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"名称", "路径", "大小", "修改时间"});
    resultsTreeView->setModel(model);
    
    mainLayout->addWidget(resultsTreeView);
    
    // 设置中央部件
    setCentralWidget(centralWidget);
    
    // 初始化状态栏
    statusBar()->setSizeGripEnabled(true);
    statusLabel = new QLabel(this);
    progressLabel = new QLabel(this);
    statusBar()->addWidget(statusLabel, 1);
    statusBar()->addWidget(progressLabel);
    
    // 设置窗口属性
    setWindowTitle("文件名搜索示例");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    // 停止任何正在进行的搜索
    if (searchEngine && searchEngine->status() == DFM::Search::SearchStatus::Running) {
        searchEngine->cancel();
    }
}

void MainWindow::setSearchType(const QString &type)
{
    searchType = type.toLower();
    
    // 更新UI
    if (searchType == "filename") {
        setWindowTitle("文件名搜索示例");
        searchTypeLabel->setText("文件名搜索");
    } else if (searchType == "content") {
        setWindowTitle("文件内容搜索示例");
        searchTypeLabel->setText("文件内容搜索");
    } else if (searchType == "app") {
        setWindowTitle("应用程序搜索示例");
        searchTypeLabel->setText("应用程序搜索");
    } else {
        // 默认为文件名搜索
        searchType = "filename";
        setWindowTitle("文件名搜索示例");
        searchTypeLabel->setText("文件名搜索");
    }
}

void MainWindow::setSearchPath(const QString &path)
{
    searchPath = path;
    pathLineEdit->setText(path);
}

void MainWindow::setUseIndex(bool useIndexFlag)
{
    useIndex = useIndexFlag;
    indexCheckBox->setChecked(useIndex);
}

void MainWindow::startSearch()
{
    // 获取查询字符串
    QString query = searchLineEdit->text();
    if (query.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入搜索关键词");
        return;
    }
    
    // 创建搜索引擎
    createSearchEngine();
    if (!searchEngine) {
        QMessageBox::critical(this, "错误", "无法创建搜索引擎");
        return;
    }
    
    // 更新状态UI
    searchButton->setEnabled(false);
    stopButton->setEnabled(true);
    pauseButton->setEnabled(true);
    pauseButton->setText("暂停");
    
    // 清空结果
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(resultsTreeView->model());
    if (model) {
        model->removeRows(0, model->rowCount());
    }
    
    // 创建搜索查询
    if (searchType == "filename") {
        searchQuery = DFM::Search::SearchQuery::createFilenameQuery(query);
    } else if (searchType == "content") {
        searchQuery = DFM::Search::SearchQuery::createContentQuery(query);
    } else if (searchType == "app") {
        searchQuery = DFM::Search::SearchQuery::createAppQuery(query);
    }
    
    // 设置搜索机制
    searchQuery.setMechanism(useIndex ? DFM::Search::SearchMechanism::Indexed : DFM::Search::SearchMechanism::Realtime);
    
    // 设置搜索选项
    searchQuery.options().setSearchPaths({searchPath});
    
    // 开始计时
    elapsedTimer.start();
    
    // 开始搜索
    updateStatusBar("正在搜索...");
    if (!searchEngine->search(searchQuery)) {
        QMessageBox::critical(this, "错误", "无法启动搜索");
        searchButton->setEnabled(true);
        stopButton->setEnabled(false);
        pauseButton->setEnabled(false);
        updateStatusBar("搜索启动失败");
    }
}

void MainWindow::stopSearch()
{
    if (searchEngine) {
        searchEngine->cancel();
        updateStatusBar("搜索已停止");
    }
    
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    pauseButton->setEnabled(false);
}

void MainWindow::togglePause()
{
    if (!searchEngine)
        return;
    
    if (searchEngine->status() == DFM::Search::SearchStatus::Running) {
        searchEngine->pause();
        pauseButton->setText("继续");
        updateStatusBar("搜索已暂停");
    } else if (searchEngine->status() == DFM::Search::SearchStatus::Paused) {
        searchEngine->resume();
        pauseButton->setText("暂停");
        updateStatusBar("搜索继续中...");
    }
}

void MainWindow::selectSearchPath()
{
    QString path = QFileDialog::getExistingDirectory(
        this, "选择搜索路径", searchPath, QFileDialog::ShowDirsOnly);
    
    if (!path.isEmpty()) {
        searchPath = path;
        pathLineEdit->setText(path);
    }
}

void MainWindow::handleSearchResults(const DFM::Search::SearchResult &results)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(resultsTreeView->model());
    if (!model)
        return;
    
    // 更新结果
    int count = results.items().size();
    model->setRowCount(count);
    
    for (int i = 0; i < count; ++i) {
        auto item = results.items()[i];
        
        if (auto fileItem = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(item)) {
            // 文件名
            model->setItem(i, 0, new QStandardItem(fileItem->displayName()));
            
            // 路径
            model->setItem(i, 1, new QStandardItem(fileItem->filePath()));
            
            // 大小
            QString sizeStr;
            if (fileItem->isDirectory()) {
                sizeStr = "<目录>";
            } else {
                qint64 size = fileItem->fileSize();
                if (size < 1024) {
                    sizeStr = QString("%1 B").arg(size);
                } else if (size < 1024 * 1024) {
                    sizeStr = QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
                } else if (size < 1024 * 1024 * 1024) {
                    sizeStr = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
                } else {
                    sizeStr = QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
                }
            }
            model->setItem(i, 2, new QStandardItem(sizeStr));
            
            // 修改时间
            model->setItem(i, 3, new QStandardItem(fileItem->lastModified().toString("yyyy-MM-dd hh:mm:ss")));
        }
    }
    
    // 更新状态栏
    QString statusText = QString("找到 %1 个结果").arg(count);
    if (results.searchTime() > 0) {
        statusText += QString(" (耗时 %1 秒)").arg(results.searchTime(), 0, 'f', 2);
    } else {
        qint64 elapsed = elapsedTimer.elapsed();
        statusText += QString(" (已用时 %1 秒)").arg(elapsed / 1000.0, 0, 'f', 2);
    }
    
    updateStatusBar(statusText);
}

void MainWindow::handleSearchProgress(int percentage)
{
    progressLabel->setText(QString("进度: %1%").arg(percentage));
}

void MainWindow::handleSearchStatus(DFM::Search::SearchStatus status)
{
    switch (status) {
        case DFM::Search::SearchStatus::Ready:
            updateStatusBar("就绪");
            break;
        case DFM::Search::SearchStatus::Running:
            updateStatusBar("搜索中...");
            break;
        case DFM::Search::SearchStatus::Paused:
            updateStatusBar("已暂停");
            break;
        case DFM::Search::SearchStatus::Completed:
            updateStatusBar(QString("搜索完成 (耗时 %1 秒)").arg(elapsedTimer.elapsed() / 1000.0, 0, 'f', 2));
            searchButton->setEnabled(true);
            stopButton->setEnabled(false);
            pauseButton->setEnabled(false);
            break;
        case DFM::Search::SearchStatus::Cancelled:
            updateStatusBar("搜索已取消");
            searchButton->setEnabled(true);
            stopButton->setEnabled(false);
            pauseButton->setEnabled(false);
            break;
        case DFM::Search::SearchStatus::Error:
            updateStatusBar("搜索出错");
            searchButton->setEnabled(true);
            stopButton->setEnabled(false);
            pauseButton->setEnabled(false);
            break;
    }
}

void MainWindow::handleSearchCompleted()
{
    qint64 elapsed = elapsedTimer.elapsed();
    updateStatusBar(QString("搜索完成 (耗时 %1 秒)").arg(elapsed / 1000.0, 0, 'f', 2));
    
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    pauseButton->setEnabled(false);
}

void MainWindow::handleSearchError(const QString &errorMessage)
{
    QMessageBox::critical(this, "搜索错误", errorMessage);
    updateStatusBar(QString("搜索错误: %1").arg(errorMessage));
    
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    pauseButton->setEnabled(false);
}

void MainWindow::handleResultDoubleClicked(const QModelIndex &index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(resultsTreeView->model());
    if (!model)
        return;
    
    // 获取路径列的值
    QModelIndex pathIndex = model->index(index.row(), 1);
    QString path = model->data(pathIndex).toString();
    
    if (!path.isEmpty()) {
        // 打开文件或目录
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void MainWindow::updateSearchOptions()
{
    // 从UI获取搜索选项
    searchOptions.setCaseSensitive(caseSensitiveCheckBox->isChecked());
    searchOptions.setUseRegex(regexCheckBox->isChecked());
    searchOptions.setMaxResults(maxResultsSpinBox->value());
    
    // 获取是否使用索引
    useIndex = indexCheckBox->isChecked();
}

void MainWindow::createSearchEngine()
{
    // 释放之前的引擎
    if (searchEngine) {
        // 断开连接
        searchEngine->disconnect(this);
        searchEngine.reset();
    }
    
    // 根据搜索类型创建引擎
    if (searchType == "filename") {
        searchEngine = DFM::Search::createFilenameSearchEngine(useIndex);
    } else if (searchType == "content") {
        searchEngine = DFM::Search::createContentSearchEngine(useIndex);
    } else if (searchType == "app") {
        // 应用程序搜索引擎
        // 这里简化处理，使用文件名搜索引擎
        searchEngine = DFM::Search::createFilenameSearchEngine(useIndex);
    }
    
    if (!searchEngine) {
        qWarning() << "无法创建搜索引擎";
        return;
    }
    
    // 连接信号
    connect(searchEngine.get(), &DFM::Search::SearchEngine::resultsReady,
            this, &MainWindow::handleSearchResults);
    connect(searchEngine.get(), &DFM::Search::SearchEngine::progressChanged,
            this, &MainWindow::handleSearchProgress);
    connect(searchEngine.get(), &DFM::Search::SearchEngine::statusChanged,
            this, &MainWindow::handleSearchStatus);
    connect(searchEngine.get(), &DFM::Search::SearchEngine::searchCompleted,
            this, &MainWindow::handleSearchCompleted);
    connect(searchEngine.get(), &DFM::Search::SearchEngine::searchError,
            this, &MainWindow::handleSearchError);
}

void MainWindow::updateStatusBar(const QString &message, int timeout)
{
    statusLabel->setText(message);
    
    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this]() {
            statusLabel->setText("就绪");
        });
    }
} 