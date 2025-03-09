#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupSearcher();
}

MainWindow::~MainWindow() { }

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 路径选择部分
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathEdit = new QLineEdit(this);
    browseButton = new QPushButton("浏览", this);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);

    // 搜索部分
    searchEdit = new QLineEdit(this);
    searchEdit->setEnabled(false);
    searchEdit->setPlaceholderText("输入搜索内容...");

    // 结果列表
    resultList = new QListWidget(this);

    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(searchEdit);
    mainLayout->addWidget(resultList);

    // 状态栏
    statusLabel = new QLabel("就绪");
    statusBar()->addWidget(statusLabel);

    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::selectDirectory);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchEditChanged);
    connect(pathEdit, &QLineEdit::textChanged, this, &MainWindow::onPathEditChanged);

    resize(800, 600);
}

void MainWindow::setupSearcher()
{
    // 创建搜索组件
    searchManager = new SearchManager(this);
    resultProcessor = new ResultProcessor(this);
    
    // 连接信号
    connect(searchManager, &SearchManager::searchResultsReady, 
            this, &MainWindow::onSearchResultsReady);
    connect(searchManager, &SearchManager::searchError,
            this, &MainWindow::onSearchError);
    connect(resultProcessor, &ResultProcessor::processingFinished,
            this, &MainWindow::onResultsProcessed);
}

void MainWindow::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择目录",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
        searchEdit->clear();
    }
}

void MainWindow::onSearchEditChanged(const QString &text)
{
    if (text.isEmpty()) {
        resultList->clear();
        searchManager->clearCache();
        updateStatusLabel("请输入搜索内容");
        return;
    }

    QString searchPath = pathEdit->text();
    if (searchPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "未设置搜索路径！");
        return;
    }

    updateStatusLabel("正在搜索...");
    searchManager->processUserInput(searchPath, text);
}

void MainWindow::onPathEditChanged(const QString &text)
{
    resultList->clear();
    searchEdit->clear();
    searchEdit->setEnabled(!text.isEmpty());
    
    if (searchManager) {
        searchManager->clearCache();
    }
    
    updateStatusLabel("就绪");
}

void MainWindow::onSearchResultsReady(const QStringList &results)
{
    updateStatusLabel(QString("找到 %1 个结果，正在处理...").arg(results.size()));
    resultProcessor->processResults(results, searchEdit->text());
}

void MainWindow::onSearchError(const QString &errorMessage)
{
    updateStatusLabel(QString("搜索错误: %1").arg(errorMessage));
    resultList->clear();
}

void MainWindow::onResultsProcessed(const QStringList &sortedResults)
{
    displayResults(sortedResults);
    updateStatusLabel(QString("显示 %1 个结果").arg(sortedResults.size()));
}

void MainWindow::updateStatusLabel(const QString &message)
{
    statusLabel->setText(message);
}

void MainWindow::displayResults(const QStringList &results)
{
    resultList->clear();
    
    // 使用虚拟项目视图优化大量结果显示
    resultList->setUniformItemSizes(true);
    
    // 批量添加项目，避免频繁的UI更新
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // 分批添加，每批1000个项目
    const int batchSize = 1000;
    for (int i = 0; i < results.size(); i += batchSize) {
        for (int j = 0; j < batchSize && i + j < results.size(); ++j) {
            resultList->addItem(results.at(i + j));
        }
        QApplication::processEvents(); // 让UI保持响应
    }
    
    QApplication::restoreOverrideCursor();
}

void MainWindow::setSearcher(SearcherInterface *searcher)
{
    if (searchManager) {
        searchManager->setSearcher(searcher);
    }
}
