#include "mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scanThread(nullptr), scanWorker(nullptr)
{
    // 设置窗口标题和大小
    setWindowTitle("智能存储空间分析与清理工具");
    resize(1000, 700);

    // 初始化界面
    setupUI();
}

MainWindow::~MainWindow()
{
    // 清理线程资源
    if (scanThread) {
        scanThread->quit();
        scanThread->wait();
    }
}

// 初始化界面
void MainWindow::setupUI()
{
    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建堆栈部件用于页面切换
    stackedWidget = new QStackedWidget(this);

    // 创建磁盘列表页面
    diskListWidget = new DiskListWidget(this);

    // 创建磁盘详情页面
    diskDetailWidget = new DiskDetailWidget(this);

    // 添加页面到堆栈部件
    stackedWidget->addWidget(diskListWidget);   // 索引0 - 首页
    stackedWidget->addWidget(diskDetailWidget);   // 索引1 - 磁盘详情页

    // 设置中央布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget);

    // 连接信号和槽
    connect(diskListWidget, &DiskListWidget::diskSelected, this, &MainWindow::enterDiskDetail);
    connect(diskDetailWidget, &DiskDetailWidget::backToHomeRequested, this, &MainWindow::backToHome);
    connect(diskDetailWidget, &DiskDetailWidget::scanRequested, this, &MainWindow::scanDiskUsage);
    connect(diskDetailWidget, &DiskDetailWidget::cleanRequested, this, &MainWindow::performCleaning);
    connect(diskDetailWidget, &DiskDetailWidget::suggestionRequested, this, &MainWindow::provideCleaningSuggestions);
}

// 扫描磁盘使用情况
void MainWindow::scanDiskUsage(const QString &mountPoint)
{
    // 创建线程和工作对象
    if (scanThread) {
        scanThread->quit();
        scanThread->wait();
    }

    scanThread = new QThread(this);
    scanWorker = new ScanWorker();

    // 将工作对象移至线程
    scanWorker->moveToThread(scanThread);

    // 连接线程信号和槽
    connect(scanThread, &QThread::started, [this, mountPoint]() {
        emit scanWorker->startScan(mountPoint);
    });

    connect(scanWorker, &ScanWorker::scanFinished, this, &MainWindow::onScanFinished);
    connect(scanWorker, &ScanWorker::largeFilesFound, this, &MainWindow::onLargeFilesFound);
    connect(scanWorker, &ScanWorker::duplicateFileFound, this, &MainWindow::onDuplicateFileFound);
    connect(scanWorker, &ScanWorker::appFileFound, this, &MainWindow::onAppFileFound);
    connect(scanWorker, &ScanWorker::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(&scanThread, &QThread::finished, scanWorker, &QObject::deleteLater);
    // 启动线程
    scanThread->start();

    // 更新可视化
    updateDiskVisualization();
}

// 处理扫描完成事件
void MainWindow::onScanFinished()
{
    // TODO: 可以在这里处理扫描完成后的操作
}

// 处理大文件扫描结果
void MainWindow::onLargeFilesFound(const QStringList &files)
{
    diskDetailWidget->addBigFiles(files);
}

// 处理重复文件扫描结果
void MainWindow::onDuplicateFileFound(const QString &file, int count)
{
    diskDetailWidget->addDuplicateFile(file, count);
}

// 处理应用文件扫描结果
void MainWindow::onAppFileFound(const QString &file, const QString &appName)
{
    diskDetailWidget->addAppFile(file, appName);
}

// 处理扫描进度更新
void MainWindow::onProgressUpdated(int progress, const QString &message)
{
    // TODO: 可以在这里更新进度条或状态信息
    qDebug() << "Scan progress:" << progress << "% -" << message;
}

// 识别大文件
void MainWindow::detectLargeFiles()
{
    // TODO: 实现大文件识别功能 (>50MB)
    QMessageBox::information(this, "提示", "正在识别大文件...");
}

// 识别应用产生的文件
void MainWindow::detectAppFiles()
{
    // TODO: 实现应用文件识别功能
    QMessageBox::information(this, "提示", "正在识别应用产生的文件...");
}

// 识别重复文件
void MainWindow::detectDuplicateFiles()
{
    // TODO: 实现重复文件识别功能
    QMessageBox::information(this, "提示", "正在识别重复文件...");
}

// 提供清理建议
void MainWindow::provideCleaningSuggestions()
{
    // TODO: 实现智能清理建议功能
    QMessageBox::information(this, "清理建议", "建议清理以下内容:\n1. 临时文件 (约2.5GB)\n2. 重复文件 (约1.2GB)\n3. 大文件 (约5.8GB)");
}

// 执行清理操作
void MainWindow::performCleaning()
{
    // TODO: 实现安全清理功能
    QMessageBox::information(this, "提示", "开始执行清理操作...");
}

// 返回首页
void MainWindow::backToHome()
{
    stackedWidget->setCurrentIndex(0);   // 切换到首页
}

// 进入磁盘详情
void MainWindow::enterDiskDetail(const QString id)
{
    stackedWidget->setCurrentIndex(1);   // 切换到磁盘详情页
    diskDetailWidget->setDiskInfo(id);
}

// 更新磁盘使用可视化
void MainWindow::updateDiskVisualization()
{
    // TODO: 实现磁盘使用情况的可视化更新
    QMessageBox::information(this, "提示", "磁盘使用情况可视化已更新");
}
