#include "OfficePreviewWidget.h"
#include "OfficeConverter.h"
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QSplitter>
#include <QDebug>
#include <QResizeEvent>
#include <QStandardPaths>

const double OfficePreviewWidget::MIN_SCALE = 0.1;
const double OfficePreviewWidget::MAX_SCALE = 5.0;
const double OfficePreviewWidget::SCALE_STEP = 0.1;

OfficePreviewWidget::OfficePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_toolBar(nullptr)
    , m_scrollArea(nullptr)
    , m_imageLabel(nullptr)
    , m_loadingLabel(nullptr)
    , m_loadingMovie(nullptr)
    , m_pdfDocument(nullptr)
    , m_currentPage(0)
    , m_scaleFactor(1.0)
    , m_converter(nullptr)
    , m_tempPdfFile(nullptr)
{
    setupUI();
    
    m_converter = new OfficeConverter(this);
    connect(m_converter, &OfficeConverter::conversionFinished,
            this, &OfficePreviewWidget::onConversionFinished);
    connect(m_converter, &OfficeConverter::conversionError,
            this, &OfficePreviewWidget::onConversionError);
    connect(m_converter, &OfficeConverter::conversionProgress,
            this, &OfficePreviewWidget::onConversionProgress);
}

OfficePreviewWidget::~OfficePreviewWidget()
{
    // 智能指针自动清理，不需要手动delete
    if (m_tempPdfFile) {
        delete m_tempPdfFile;
    }
}

void OfficePreviewWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupToolBar();
    
    // 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setBackgroundRole(QPalette::Mid);
    
    // 创建图像标签
    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setMinimumSize(400, 300);
    m_imageLabel->setText("请选择Office文件进行预览\n\n支持格式:\n• Word (.docx, .doc)\n• Excel (.xlsx, .xls)\n• PowerPoint (.pptx, .ppt)");
    m_imageLabel->setStyleSheet("color: #888; font-size: 14px; padding: 20px;");
    
    m_scrollArea->setWidget(m_imageLabel);
    
    // 创建加载动画
    m_loadingLabel = new QLabel(this);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setText("正在转换文件，请稍候...");
    m_loadingLabel->setVisible(false);
    m_loadingLabel->setStyleSheet("background-color: rgba(0,0,0,180); color: white; font-size: 16px; padding: 20px; border-radius: 10px;");
    
    m_mainLayout->addWidget(m_toolBar);
    m_mainLayout->addWidget(m_scrollArea, 1);
}

void OfficePreviewWidget::setupToolBar()
{
    m_toolBar = new QToolBar(this);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // 页面导航
    m_prevPageButton = new QPushButton("◀ 上一页", this);
    m_prevPageButton->setEnabled(false);
    connect(m_prevPageButton, &QPushButton::clicked, this, &OfficePreviewWidget::previousPage);
    
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setMinimum(1);
    m_pageSpinBox->setEnabled(false);
    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OfficePreviewWidget::onPageSpinBoxChanged);
    
    m_pageCountLabel = new QLabel("/ 0", this);
    
    m_nextPageButton = new QPushButton("下一页 ▶", this);
    m_nextPageButton->setEnabled(false);
    connect(m_nextPageButton, &QPushButton::clicked, this, &OfficePreviewWidget::nextPage);
    
    // 缩放控制
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setMinimum(static_cast<int>(MIN_SCALE * 100));
    m_zoomSlider->setMaximum(static_cast<int>(MAX_SCALE * 100));
    m_zoomSlider->setValue(100);
    m_zoomSlider->setEnabled(false);
    m_zoomSlider->setMaximumWidth(150);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &OfficePreviewWidget::onZoomSliderChanged);
    
    m_zoomLabel = new QLabel("100%", this);
    m_zoomLabel->setMinimumWidth(50);
    
    m_zoomFitButton = new QPushButton("适应窗口", this);
    m_zoomFitButton->setEnabled(false);
    connect(m_zoomFitButton, &QPushButton::clicked, this, &OfficePreviewWidget::zoomFit);
    
    m_zoomActualButton = new QPushButton("实际大小", this);
    m_zoomActualButton->setEnabled(false);
    connect(m_zoomActualButton, &QPushButton::clicked, this, &OfficePreviewWidget::zoomActual);
    
    // 添加到工具栏
    m_toolBar->addWidget(m_prevPageButton);
    m_toolBar->addWidget(m_pageSpinBox);
    m_toolBar->addWidget(m_pageCountLabel);
    m_toolBar->addWidget(m_nextPageButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(new QLabel("缩放:"));
    m_toolBar->addWidget(m_zoomSlider);
    m_toolBar->addWidget(m_zoomLabel);
    m_toolBar->addWidget(m_zoomFitButton);
    m_toolBar->addWidget(m_zoomActualButton);
}

void OfficePreviewWidget::previewFile(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        emit previewError("文件不存在: " + filePath);
        return;
    }
    
    // 清理之前的文档 - 智能指针自动清理
    m_pdfDocument.reset();
    
    if (m_tempPdfFile) {
        delete m_tempPdfFile;
        m_tempPdfFile = nullptr;
    }
    
    showLoadingAnimation();
    
    // 创建临时PDF文件
    m_tempPdfFile = new QTemporaryFile(this);
    m_tempPdfFile->setFileTemplate(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/office_preview_XXXXXX.pdf");
    
    if (!m_tempPdfFile->open()) {
        emit previewError("无法创建临时文件");
        return;
    }
    
    QString outputPath = m_tempPdfFile->fileName();
    m_tempPdfFile->close();
    
    // 开始转换
    m_converter->convertToPdf(filePath, outputPath);
}

void OfficePreviewWidget::onConversionFinished(const QString &pdfPath)
{
    hideLoadingAnimation();
    loadPdf(pdfPath);
    emit previewProgress(100);
}

void OfficePreviewWidget::onConversionError(const QString &error)
{
    hideLoadingAnimation();
    emit previewError(error);
}

void OfficePreviewWidget::onConversionProgress(int percentage)
{
    emit previewProgress(percentage);
}

void OfficePreviewWidget::loadPdf(const QString &pdfPath)
{
    if (!QFile::exists(pdfPath)) {
        emit previewError("PDF文件不存在: " + pdfPath);
        return;
    }
    
    // 加载PDF文档 - 使用智能指针move语义
    m_pdfDocument = std::move(Poppler::Document::load(pdfPath));
    
    if (!m_pdfDocument || m_pdfDocument->isLocked()) {
        emit previewError("无法打开PDF文件或文件已加密");
        return;
    }
    
    // 设置渲染提示
    m_pdfDocument->setRenderHint(Poppler::Document::Antialiasing);
    m_pdfDocument->setRenderHint(Poppler::Document::TextAntialiasing);
    
    m_currentPage = 0;
    
    // 更新UI状态
    int pageCount = m_pdfDocument->numPages();
    m_pageSpinBox->setMaximum(pageCount);
    m_pageSpinBox->setValue(1);
    m_pageSpinBox->setEnabled(pageCount > 0);
    m_pageCountLabel->setText(QString("/ %1").arg(pageCount));
    
    m_prevPageButton->setEnabled(false);
    m_nextPageButton->setEnabled(pageCount > 1);
    
    m_zoomSlider->setEnabled(true);
    m_zoomFitButton->setEnabled(true);
    m_zoomActualButton->setEnabled(true);
    
    // 渲染当前页面
    renderCurrentPage();
}

void OfficePreviewWidget::renderCurrentPage()
{
    if (!m_pdfDocument || m_currentPage < 0 || m_currentPage >= m_pdfDocument->numPages()) {
        return;
    }
    
    // 使用智能指针
    std::unique_ptr<Poppler::Page> page = m_pdfDocument->page(m_currentPage);
    if (!page) {
        return;
    }
    
    // 计算DPI
    double dpi = 72.0 * m_scaleFactor;
    
    // 渲染页面
    QImage image = page->renderToImage(dpi, dpi);
    if (image.isNull()) {
        emit previewError("渲染PDF页面失败");
        return;
    }
    
    m_currentPixmap = QPixmap::fromImage(image);
    m_imageLabel->setPixmap(m_currentPixmap);
    m_imageLabel->resize(m_currentPixmap.size());
    
    // 智能指针自动清理，不需要手动delete
    
    updatePageInfo();
    updateZoomInfo();
}

void OfficePreviewWidget::updatePageInfo()
{
    if (m_pdfDocument) {
        m_prevPageButton->setEnabled(m_currentPage > 0);
        m_nextPageButton->setEnabled(m_currentPage < m_pdfDocument->numPages() - 1);
        m_pageSpinBox->setValue(m_currentPage + 1);
    }
}

void OfficePreviewWidget::updateZoomInfo()
{
    m_zoomSlider->setValue(static_cast<int>(m_scaleFactor * 100));
    m_zoomLabel->setText(QString("%1%").arg(static_cast<int>(m_scaleFactor * 100)));
}

void OfficePreviewWidget::zoomIn()
{
    m_scaleFactor = qMin(m_scaleFactor + SCALE_STEP, MAX_SCALE);
    renderCurrentPage();
}

void OfficePreviewWidget::zoomOut()
{
    m_scaleFactor = qMax(m_scaleFactor - SCALE_STEP, MIN_SCALE);
    renderCurrentPage();
}

void OfficePreviewWidget::zoomFit()
{
    if (!m_pdfDocument || m_currentPage < 0 || m_currentPage >= m_pdfDocument->numPages()) {
        return;
    }
    
    // 使用智能指针
    std::unique_ptr<Poppler::Page> page = m_pdfDocument->page(m_currentPage);
    if (!page) {
        return;
    }
    
    QSizeF pageSize = page->pageSizeF();
    QSize viewportSize = m_scrollArea->viewport()->size();
    
    double scaleX = static_cast<double>(viewportSize.width() - 20) / pageSize.width();
    double scaleY = static_cast<double>(viewportSize.height() - 20) / pageSize.height();
    
    m_scaleFactor = qMin(scaleX, scaleY);
    m_scaleFactor = qBound(MIN_SCALE, m_scaleFactor, MAX_SCALE);
    
    // 智能指针自动清理，不需要手动delete
    renderCurrentPage();
}

void OfficePreviewWidget::zoomActual()
{
    m_scaleFactor = 1.0;
    renderCurrentPage();
}

void OfficePreviewWidget::onZoomSliderChanged(int value)
{
    m_scaleFactor = value / 100.0;
    renderCurrentPage();
}

void OfficePreviewWidget::previousPage()
{
    if (m_currentPage > 0) {
        m_currentPage--;
        renderCurrentPage();
    }
}

void OfficePreviewWidget::nextPage()
{
    if (m_pdfDocument && m_currentPage < m_pdfDocument->numPages() - 1) {
        m_currentPage++;
        renderCurrentPage();
    }
}

void OfficePreviewWidget::onPageSpinBoxChanged(int page)
{
    if (m_pdfDocument && page > 0 && page <= m_pdfDocument->numPages()) {
        m_currentPage = page - 1;
        renderCurrentPage();
    }
}

void OfficePreviewWidget::onPageChanged()
{
    renderCurrentPage();
}

void OfficePreviewWidget::showLoadingAnimation()
{
    m_loadingLabel->setVisible(true);
    m_loadingLabel->raise();
    
    // 调整位置到中心
    QSize parentSize = size();
    QSize labelSize = m_loadingLabel->sizeHint();
    m_loadingLabel->move((parentSize.width() - labelSize.width()) / 2,
                        (parentSize.height() - labelSize.height()) / 2);
}

void OfficePreviewWidget::hideLoadingAnimation()
{
    m_loadingLabel->setVisible(false);
}

void OfficePreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 重新调整加载标签位置
    if (m_loadingLabel && m_loadingLabel->isVisible()) {
        QSize parentSize = size();
        QSize labelSize = m_loadingLabel->sizeHint();
        m_loadingLabel->move((parentSize.width() - labelSize.width()) / 2,
                            (parentSize.height() - labelSize.height()) / 2);
    }
}