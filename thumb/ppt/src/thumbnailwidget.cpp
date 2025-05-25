#include "thumbnailwidget.h"
#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

ThumbnailWidget::ThumbnailWidget(const QString &imagePath, int pageNumber, QWidget *parent)
    : QFrame(parent)
    , m_imagePath(imagePath)
    , m_pageNumber(pageNumber)
    , m_layout(nullptr)
    , m_imageLabel(nullptr)
    , m_pageLabel(nullptr)
    , m_isHovered(false)
{
    setupUI();
    setThumbnailImage(imagePath);
}

void ThumbnailWidget::setupUI()
{
    setFrameStyle(QFrame::Box);
    setLineWidth(2);
    setMidLineWidth(0);
    
    // 设置基本样式
    setStyleSheet(
        "ThumbnailWidget {"
        "    background-color: #3c3c3c;"
        "    border: 2px solid #555555;"
        "    border-radius: 8px;"
        "}"
        "ThumbnailWidget:hover {"
        "    border-color: #0078d4;"
        "    background-color: #404040;"
        "}"
    );
    
    setFixedSize(THUMBNAIL_WIDTH + 20, THUMBNAIL_HEIGHT + 50);
    setCursor(Qt::PointingHandCursor);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(5);
    
    // 图像标签
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(false);
    m_imageLabel->setMinimumSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    m_imageLabel->setMaximumSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    m_imageLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #2b2b2b;"
        "    border: 1px solid #555555;"
        "    border-radius: 5px;"
        "}"
    );
    
    // 页码标签
    m_pageLabel = new QLabel(QString("第 %1 页").arg(m_pageNumber), this);
    m_pageLabel->setAlignment(Qt::AlignCenter);
    m_pageLabel->setStyleSheet(
        "QLabel {"
        "    color: #ffffff;"
        "    background-color: transparent;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "    border: none;"
        "}"
    );
    
    m_layout->addWidget(m_imageLabel);
    m_layout->addWidget(m_pageLabel);
    
    // 添加阴影效果
    auto *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(10);
    shadowEffect->setColor(QColor(0, 0, 0, 80));
    shadowEffect->setOffset(2, 2);
    setGraphicsEffect(shadowEffect);
}

void ThumbnailWidget::setThumbnailImage(const QString &imagePath)
{
    m_imagePath = imagePath;
    
    if (!QFileInfo::exists(imagePath)) {
        qWarning() << "Thumbnail image does not exist:" << imagePath;
        
        // 设置占位图像
        QPixmap placeholder(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
        placeholder.fill(QColor("#404040"));
        
        QPainter painter(&placeholder);
        painter.setPen(QPen(QColor("#cccccc"), 2));
        painter.setFont(QFont("Arial", 12));
        painter.drawText(placeholder.rect(), Qt::AlignCenter, "无法\n加载\n图像");
        
        m_imageLabel->setPixmap(placeholder);
        return;
    }
    
    updateThumbnailDisplay();
}

void ThumbnailWidget::updateThumbnailDisplay()
{
    if (m_imagePath.isEmpty()) {
        return;
    }
    
    QPixmap originalPixmap(m_imagePath);
    if (originalPixmap.isNull()) {
        qWarning() << "Failed to load thumbnail image:" << m_imagePath;
        return;
    }
    
    // 缩放图像保持宽高比
    QPixmap scaledPixmap = originalPixmap.scaled(
        THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    
    // 如果需要，创建一个居中的图像
    if (scaledPixmap.size() != QSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT)) {
        QPixmap centeredPixmap(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
        centeredPixmap.fill(QColor("#2b2b2b"));
        
        QPainter painter(&centeredPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        int x = (THUMBNAIL_WIDTH - scaledPixmap.width()) / 2;
        int y = (THUMBNAIL_HEIGHT - scaledPixmap.height()) / 2;
        
        painter.drawPixmap(x, y, scaledPixmap);
        
        m_thumbnail = centeredPixmap;
    } else {
        m_thumbnail = scaledPixmap;
    }
    
    m_imageLabel->setPixmap(m_thumbnail);
}

void ThumbnailWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_pageNumber);
    }
    QFrame::mouseReleaseEvent(event);
}

void ThumbnailWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked(m_pageNumber);
    }
    QFrame::mouseDoubleClickEvent(event);
}

void ThumbnailWidget::enterEvent(QEnterEvent *event)
{
    setHoverState(true);
    QFrame::enterEvent(event);
}

void ThumbnailWidget::leaveEvent(QEvent *event)
{
    setHoverState(false);
    QFrame::leaveEvent(event);
}

void ThumbnailWidget::setHoverState(bool hovered)
{
    if (m_isHovered == hovered) {
        return;
    }
    
    m_isHovered = hovered;
    
    // 更新样式
    if (m_isHovered) {
        setStyleSheet(
            "ThumbnailWidget {"
            "    background-color: #404040;"
            "    border: 2px solid #0078d4;"
            "    border-radius: 8px;"
            "}"
        );
        
        // 更新阴影效果
        auto *shadowEffect = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect());
        if (shadowEffect) {
            shadowEffect->setBlurRadius(15);
            shadowEffect->setColor(QColor(0, 120, 212, 120));
            shadowEffect->setOffset(0, 0);
        }
    } else {
        setStyleSheet(
            "ThumbnailWidget {"
            "    background-color: #3c3c3c;"
            "    border: 2px solid #555555;"
            "    border-radius: 8px;"
            "}"
        );
        
        // 恢复原始阴影效果
        auto *shadowEffect = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect());
        if (shadowEffect) {
            shadowEffect->setBlurRadius(10);
            shadowEffect->setColor(QColor(0, 0, 0, 80));
            shadowEffect->setOffset(2, 2);
        }
    }
    
    update();
}

void ThumbnailWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    
    // 如果需要额外的自定义绘制，可以在这里添加
    if (m_isHovered) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制边框高亮
        QPen pen(QColor("#0078d4"), 3);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        
        QRect borderRect = rect().adjusted(1, 1, -1, -1);
        painter.drawRoundedRect(borderRect, BORDER_RADIUS, BORDER_RADIUS);
    }
} 