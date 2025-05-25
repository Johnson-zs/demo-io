#include "mainwindow.h"
#include "thumbnailgenerator.h"
#include "thumbnailwidget.h"
#include <QFileInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_splitter(nullptr)
    , m_mainLayout(nullptr)
    , m_toolbarLayout(nullptr)
    , m_openButton(nullptr)
    , m_clearButton(nullptr)
    , m_fileInfoLabel(nullptr)
    , m_pageCountLabel(nullptr)
    , m_methodComboBox(nullptr)
    , m_methodLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_thumbnailContainer(nullptr)
    , m_thumbnailLayout(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_pageLabel(nullptr)
    , m_generator(nullptr)
    , m_totalPages(0)
    , m_currentPage(0)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    // 创建缩略图生成器
    m_generator = new ThumbnailGenerator(this);
    connect(m_generator, &ThumbnailGenerator::thumbnailGenerated,
            this, &MainWindow::onThumbnailGenerated);
    connect(m_generator, &ThumbnailGenerator::generationFinished,
            this, &MainWindow::onGenerationFinished);
    connect(m_generator, &ThumbnailGenerator::generationError,
            this, &MainWindow::onGenerationError);
    connect(m_generator, &ThumbnailGenerator::methodUsed,
            this, &MainWindow::onMethodUsed);
    
    updateStatusMessage("就绪");
    setWindowTitle("PPT缩略图查看器");
    resize(1000, 700);
}

MainWindow::~MainWindow()
{
    if (m_generator) {
        m_generator->stopGeneration();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 工具栏布局
    m_toolbarLayout = new QHBoxLayout();
    
    m_openButton = new QPushButton("打开PPT文件", this);
    m_openButton->setMinimumHeight(40);
    m_openButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #0078d4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005a9e;"
        "}"
    );
    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    
    m_clearButton = new QPushButton("清除", this);
    m_clearButton->setMinimumHeight(40);
    m_clearButton->setEnabled(false);
    m_clearButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #d13438;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover:enabled {"
        "    background-color: #a4262c;"
        "}"
        "QPushButton:pressed:enabled {"
        "    background-color: #8e2227;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #555555;"
        "    color: #888888;"
        "}"
    );
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearThumbnails);
    
    m_fileInfoLabel = new QLabel("未选择文件", this);
    m_fileInfoLabel->setStyleSheet("QLabel { color: #cccccc; font-size: 12px; }");
    
    m_pageCountLabel = new QLabel("", this);
    m_pageCountLabel->setStyleSheet("QLabel { color: #cccccc; font-size: 12px; }");
    
    // 生成方式选择
    m_methodLabel = new QLabel("生成方式:", this);
    m_methodLabel->setStyleSheet("QLabel { color: #cccccc; font-size: 12px; }");
    
    m_methodComboBox = new QComboBox(this);
    m_methodComboBox->addItem("自动检测", static_cast<int>(ThumbnailGenerator::AutoDetect));
    m_methodComboBox->addItem("KDE提取", static_cast<int>(ThumbnailGenerator::KDEExtraction));
    m_methodComboBox->addItem("LibreOffice转换", static_cast<int>(ThumbnailGenerator::LibreOfficeConversion));
    m_methodComboBox->setMinimumHeight(35);
    m_methodComboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: #3c3c3c;"
        "    color: #ffffff;"
        "    border: 2px solid #555555;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "    font-size: 12px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border: none;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #3c3c3c;"
        "    color: #ffffff;"
        "    border: 1px solid #555555;"
        "    selection-background-color: #0078d4;"
        "}"
    );
    connect(m_methodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onGenerationMethodChanged);
    
    m_toolbarLayout->addWidget(m_openButton);
    m_toolbarLayout->addWidget(m_clearButton);
    m_toolbarLayout->addWidget(m_fileInfoLabel);
    m_toolbarLayout->addStretch();
    m_toolbarLayout->addWidget(m_methodLabel);
    m_toolbarLayout->addWidget(m_methodComboBox);
    m_toolbarLayout->addWidget(m_pageCountLabel);
    
    m_mainLayout->addLayout(m_toolbarLayout);
    
    // 缩略图显示区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: 2px solid #404040;"
        "    border-radius: 5px;"
        "    background-color: #2b2b2b;"
        "}"
    );
    
    m_thumbnailContainer = new QWidget();
    m_thumbnailLayout = new QGridLayout(m_thumbnailContainer);
    m_thumbnailLayout->setSpacing(15);
    m_thumbnailLayout->setContentsMargins(15, 15, 15, 15);
    
    m_scrollArea->setWidget(m_thumbnailContainer);
    m_mainLayout->addWidget(m_scrollArea);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid #404040;"
        "    border-radius: 5px;"
        "    text-align: center;"
        "    background-color: #2b2b2b;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #0078d4;"
        "    border-radius: 3px;"
        "}"
    );
    m_mainLayout->addWidget(m_progressBar);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("文件(&F)");
    
    auto *openAction = fileMenu->addAction("打开PPT文件(&O)");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    fileMenu->addSeparator();
    
    auto *clearAction = fileMenu->addAction("清除缩略图(&C)");
    clearAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearThumbnails);
    
    fileMenu->addSeparator();
    
    auto *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    auto *helpMenu = menuBar()->addMenu("帮助(&H)");
    auto *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(m_statusLabel);
    
    m_pageLabel = new QLabel("", this);
    statusBar()->addPermanentWidget(m_pageLabel);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择PPT文件",
        QString(),
        "PowerPoint文件 (*.ppt *.pptx);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        clearThumbnails();
        
        QFileInfo fileInfo(fileName);
        m_currentFile = fileName;
        m_fileInfoLabel->setText(fileInfo.fileName());
        
        updateStatusMessage("正在生成缩略图...");
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0); // 无限进度条
        
        m_openButton->setEnabled(false);
        
        m_generator->generateThumbnails(fileName);
    }
}

void MainWindow::clearThumbnails()
{
    // 清除所有缩略图组件
    for (auto *widget : m_thumbnailWidgets) {
        m_thumbnailLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_thumbnailWidgets.clear();
    
    m_currentFile.clear();
    m_totalPages = 0;
    m_currentPage = 0;
    
    m_fileInfoLabel->setText("未选择文件");
    m_pageCountLabel->setText("");
    m_clearButton->setEnabled(false);
    
    updateStatusMessage("就绪");
}

void MainWindow::onThumbnailGenerated(const QString &imagePath, int pageNumber)
{
    auto *thumbnailWidget = new ThumbnailWidget(imagePath, pageNumber, this);
    
    // 计算网格位置 (每行4个缩略图)
    int row = (pageNumber - 1) / 4;
    int col = (pageNumber - 1) % 4;
    
    m_thumbnailLayout->addWidget(thumbnailWidget, row, col);
    m_thumbnailWidgets.append(thumbnailWidget);
    
    // 连接点击事件
    connect(thumbnailWidget, &ThumbnailWidget::clicked, [this](int page) {
        updateStatusMessage(QString("选中第 %1 页").arg(page));
    });
    
    connect(thumbnailWidget, &ThumbnailWidget::doubleClicked, [this](int page) {
        updateStatusMessage(QString("双击第 %1 页").arg(page));
        // 这里可以添加双击处理逻辑，比如全屏显示
    });
    
    m_currentPage = pageNumber;
    m_pageLabel->setText(QString("第 %1 页").arg(pageNumber));
    
    if (m_totalPages == 0) {
        m_totalPages = pageNumber; // 暂时设置，稍后会更新
    }
}

void MainWindow::onGenerationFinished()
{
    m_progressBar->setVisible(false);
    m_openButton->setEnabled(true);
    m_clearButton->setEnabled(true);
    
    m_totalPages = m_thumbnailWidgets.size();
    m_pageCountLabel->setText(QString("共 %1 页").arg(m_totalPages));
    
    updateStatusMessage(QString("成功生成 %1 个缩略图").arg(m_totalPages));
}

void MainWindow::onGenerationError(const QString &error)
{
    m_progressBar->setVisible(false);
    m_openButton->setEnabled(true);
    
    QMessageBox::critical(this, "错误", QString("生成缩略图失败：\n%1").arg(error));
    updateStatusMessage("生成失败");
}

void MainWindow::updateStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void MainWindow::onMethodUsed(const QString &methodName)
{
    updateStatusMessage(QString("使用 %1 生成缩略图").arg(methodName));
}

void MainWindow::onGenerationMethodChanged()
{
    if (!m_generator) return;
    
    int methodIndex = m_methodComboBox->currentData().toInt();
    auto method = static_cast<ThumbnailGenerator::GenerationMethod>(methodIndex);
    m_generator->setGenerationMethod(method);
    
    QString methodName = m_methodComboBox->currentText();
    qDebug() << "Changed generation method to:" << methodName;
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于",
        "<h3>PPT缩略图查看器</h3>"
        "<p>版本 2.0</p>"
        "<p>基于Qt6开发的PowerPoint文件缩略图生成和查看工具。</p>"
        "<p><b>支持两种生成方式：</b></p>"
        "<ul>"
        "<li><b>KDE提取</b>：直接从文件提取内嵌缩略图（速度极快）</li>"
        "<li><b>LibreOffice转换</b>：使用LibreOffice转换（兼容性好）</li>"
        "<li><b>自动检测</b>：优先KDE方式，失败则使用LibreOffice</li>"
        "</ul>"
        "<p><b>功能特性：</b></p>"
        "<ul>"
        "<li>支持.ppt和.pptx格式</li>"
        "<li>高质量缩略图生成</li>"
        "<li>现代化界面设计</li>"
        "<li>快速预览和浏览</li>"
        "<li>多种生成方式可选</li>"
        "</ul>"
    );
} 