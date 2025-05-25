#include "MainWindow.h"
#include "OfficePreviewWidget.h"
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_recentFilesTree(nullptr)
    , m_previewWidget(nullptr)
    , m_openButton(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
{
    setWindowTitle("Office Preview - 快速预览工具");
    setMinimumSize(1000, 700);
    resize(1400, 900);
    
    // 启用拖拽
    setAcceptDrops(true);
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    loadRecentFiles();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建左侧面板 - 最近文件列表
    m_recentFilesTree = new QTreeWidget(this);
    m_recentFilesTree->setHeaderLabel("最近文件");
    m_recentFilesTree->setMaximumWidth(300);
    m_recentFilesTree->setMinimumWidth(200);
    m_recentFilesTree->header()->setStretchLastSection(true);
    
    // 连接最近文件点击事件
    connect(m_recentFilesTree, &QTreeWidget::itemDoubleClicked, 
            [this](QTreeWidgetItem *item) {
                if (item && item->data(0, Qt::UserRole).isValid()) {
                    QString filePath = item->data(0, Qt::UserRole).toString();
                    if (QFile::exists(filePath)) {
                        onFileOpened(filePath);
                    } else {
                        QMessageBox::warning(this, "文件不存在", 
                                           QString("文件 %1 不存在，可能已被删除或移动").arg(filePath));
                    }
                }
            });
    
    // 创建预览组件
    m_previewWidget = new OfficePreviewWidget(this);
    
    // 连接预览组件信号
    connect(m_previewWidget, &OfficePreviewWidget::previewProgress,
            this, &MainWindow::onPreviewProgress);
    connect(m_previewWidget, &OfficePreviewWidget::previewError,
            this, &MainWindow::onPreviewError);
    
    // 添加到分割器
    m_mainSplitter->addWidget(m_recentFilesTree);
    m_mainSplitter->addWidget(m_previewWidget);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    
    // 设置布局
    QHBoxLayout *layout = new QHBoxLayout(m_centralWidget);
    layout->addWidget(m_mainSplitter);
    layout->setContentsMargins(5, 5, 5, 5);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    m_fileMenu = menuBar()->addMenu("文件(&F)");
    
    m_openAction = m_fileMenu->addAction("打开文件(&O)");
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = m_fileMenu->addAction("退出(&X)");
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 帮助菜单
    m_helpMenu = menuBar()->addMenu("帮助(&H)");
    
    m_aboutAction = m_helpMenu->addAction("关于(&A)");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    
    m_openButton = new QPushButton("打开文件", this);
    m_openButton->setMinimumHeight(32);
    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    
    toolBar->addWidget(m_openButton);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("就绪", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::openFile()
{
    QString filter = "Office文件 (*.docx *.doc *.xlsx *.xls *.pptx *.ppt)";
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择Office文件", QDir::homePath(), filter);
    
    if (!fileName.isEmpty()) {
        onFileOpened(fileName);
    }
}

void MainWindow::onFileOpened(const QString &filePath)
{
    if (!isOfficeFile(filePath)) {
        QMessageBox::warning(this, "不支持的文件", 
                           "请选择支持的Office文件格式：\n"
                           "Word: .docx, .doc\n"
                           "Excel: .xlsx, .xls\n"
                           "PowerPoint: .pptx, .ppt");
        return;
    }
    
    updateRecentFiles(filePath);
    m_statusLabel->setText(QString("正在预览: %1").arg(QFileInfo(filePath).fileName()));
    m_previewWidget->previewFile(filePath);
}

void MainWindow::onPreviewProgress(int percentage)
{
    m_progressBar->setVisible(percentage < 100);
    m_progressBar->setValue(percentage);
    
    if (percentage >= 100) {
        m_statusLabel->setText("预览完成");
    } else {
        m_statusLabel->setText(QString("正在处理... %1%").arg(percentage));
    }
}

void MainWindow::onPreviewError(const QString &error)
{
    m_progressBar->setVisible(false);
    m_statusLabel->setText("预览失败");
    QMessageBox::critical(this, "预览错误", error);
}

void MainWindow::updateRecentFiles(const QString &filePath)
{
    // 移除已存在的文件路径
    m_recentFiles.removeAll(filePath);
    
    // 添加到开头
    m_recentFiles.prepend(filePath);
    
    // 限制数量
    while (m_recentFiles.size() > MAX_RECENT_FILES) {
        m_recentFiles.removeLast();
    }
    
    // 保存到设置
    QSettings settings;
    settings.setValue("recentFiles", m_recentFiles);
    
    // 更新UI
    m_recentFilesTree->clear();
    for (const QString &file : m_recentFiles) {
        if (QFile::exists(file)) {
            QFileInfo fileInfo(file);
            QTreeWidgetItem *item = new QTreeWidgetItem(m_recentFilesTree);
            item->setText(0, fileInfo.fileName());
            item->setToolTip(0, file);
            item->setData(0, Qt::UserRole, file);
            
            // 根据文件类型设置图标
            if (file.endsWith(".docx") || file.endsWith(".doc")) {
                item->setText(0, "📄 " + fileInfo.fileName());
            } else if (file.endsWith(".xlsx") || file.endsWith(".xls")) {
                item->setText(0, "📊 " + fileInfo.fileName());
            } else if (file.endsWith(".pptx") || file.endsWith(".ppt")) {
                item->setText(0, "📊 " + fileInfo.fileName());
            }
        }
    }
}

void MainWindow::loadRecentFiles()
{
    QSettings settings;
    m_recentFiles = settings.value("recentFiles").toStringList();
    
    // 过滤不存在的文件
    QStringList validFiles;
    for (const QString &file : m_recentFiles) {
        if (QFile::exists(file)) {
            validFiles.append(file);
        }
    }
    m_recentFiles = validFiles;
    
    // 更新UI
    updateRecentFiles("");
    if (!m_recentFiles.isEmpty()) {
        updateRecentFiles(m_recentFiles.first());
    }
}

bool MainWindow::isOfficeFile(const QString &filePath) const
{
    QStringList supportedExtensions = {
        ".docx", ".doc", ".xlsx", ".xls", ".pptx", ".ppt"
    };
    
    QString extension = QFileInfo(filePath).suffix().toLower();
    return supportedExtensions.contains("." + extension);
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于 Office Preview",
                      "<h3>Office Preview v1.0.0</h3>"
                      "<p>一个快速的Office文件预览工具</p>"
                      "<p>支持的格式:</p>"
                      "<ul>"
                      "<li>Microsoft Word (.docx, .doc)</li>"
                      "<li>Microsoft Excel (.xlsx, .xls)</li>"
                      "<li>Microsoft PowerPoint (.pptx, .ppt)</li>"
                      "</ul>"
                      "<p>基于 Qt6 + LibreOffice + Poppler 构建</p>");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (isOfficeFile(filePath)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        if (isOfficeFile(filePath)) {
            onFileOpened(filePath);
            event->acceptProposedAction();
        }
    }
} 