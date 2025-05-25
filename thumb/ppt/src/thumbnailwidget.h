#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

class ThumbnailWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ThumbnailWidget(const QString &imagePath, int pageNumber, QWidget *parent = nullptr);
    
    void setThumbnailImage(const QString &imagePath);
    int getPageNumber() const { return m_pageNumber; }
    QString getImagePath() const { return m_imagePath; }

signals:
    void clicked(int pageNumber);
    void doubleClicked(int pageNumber);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();
    void updateThumbnailDisplay();
    void setHoverState(bool hovered);

    QString m_imagePath;
    int m_pageNumber;
    
    QVBoxLayout *m_layout;
    QLabel *m_imageLabel;
    QLabel *m_pageLabel;
    
    QPixmap m_thumbnail;
    bool m_isHovered;
    
    static constexpr int THUMBNAIL_WIDTH = 200;
    static constexpr int THUMBNAIL_HEIGHT = 150;
    static constexpr int BORDER_RADIUS = 8;
};

#endif // THUMBNAILWIDGET_H 