#ifndef FILEICONDELEGATE_H
#define FILEICONDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QEvent>
#include <QAbstractItemModel>

/**
 * @brief Custom delegate for rendering file items and group headers in icon mode
 * 
 * This delegate handles the custom painting of both group headers (full-width) 
 * and individual file items (as icons with text below) similar to Windows Explorer icon view.
 */
class FileIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FileIconDelegate(QObject* parent = nullptr);
    ~FileIconDelegate() override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    void paintGroupHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paintFileItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QRect getExpandCollapseButtonRect(const QStyleOptionViewItem& option) const;
    bool isPointInExpandCollapseButton(const QPoint& point, const QStyleOptionViewItem& option) const;

private:
    static const int GroupHeaderHeight = 28;
    static const int FileItemSize = 80;
    static const int FileItemSpacing = 10;
    static const int IconSize = 48;
    
    mutable int m_viewWidth;
};

#endif // FILEICONDELEGATE_H 