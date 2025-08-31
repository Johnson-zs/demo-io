#include "fileviewdelegate.h"
#include "fileviewmodel.h"
#include <QApplication>
#include <QStyle>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QMouseEvent>

FileViewDelegate::FileViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FileViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;
        
    ItemType type = static_cast<ItemType>(index.data(Qt::UserRole).toInt());
    
    if (type == ItemType::GroupHeader) {
        paintGroupHeader(painter, option, index);
    } else {
        paintFileItem(painter, option, index);
    }
}

QSize FileViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return QSize();
        
    ItemType type = static_cast<ItemType>(index.data(Qt::UserRole).toInt());
    
    if (type == ItemType::GroupHeader) {
        // Group headers should span the full width - use a very large width to force full row
        return QSize(8888, GroupHeaderHeight);
    } else {
        return QSize(FileItemSize, FileItemSize + 20); // Extra space for text
    }
}

bool FileViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        
        ItemType type = static_cast<ItemType>(index.data(Qt::UserRole).toInt());
        if (type == ItemType::GroupHeader) {
            QRect expandRect = getExpandCollapseRect(option);
            if (expandRect.contains(mouseEvent->pos())) {
                FileViewModel *fileModel = qobject_cast<FileViewModel*>(model);
                if (fileModel) {
                    fileModel->toggleGroupExpansion(index.row());
                    return true;
                }
            }
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void FileViewDelegate::paintGroupHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    
    // Get the actual widget width for full-width rendering
    QRect fullRect = option.rect;
    if (option.widget) {
        fullRect.setWidth(option.widget->width() - 20);
    }
    
    // Draw background
    QColor bgColor = (option.state & QStyle::State_MouseOver) ? 
                     QColor(235, 235, 235) : QColor(248, 248, 248);
    painter->fillRect(fullRect, bgColor);
    
    // Draw expand/collapse triangle
    QRect expandRect = getExpandCollapseRect(option);
    bool isExpanded = index.data(Qt::UserRole + 2).toBool();
    
    painter->setPen(QPen(QColor(100, 100, 100), 1));
    painter->setBrush(QBrush(QColor(100, 100, 100)));
    
    QPolygon triangle;
    if (isExpanded) {
        // Down arrow (expanded)
        triangle << QPoint(expandRect.left() + 2, expandRect.top() + 4)
                 << QPoint(expandRect.right() - 2, expandRect.top() + 4)
                 << QPoint(expandRect.center().x(), expandRect.bottom() - 4);
    } else {
        // Right arrow (collapsed)
        triangle << QPoint(expandRect.left() + 4, expandRect.top() + 2)
                 << QPoint(expandRect.left() + 4, expandRect.bottom() - 2)
                 << QPoint(expandRect.right() - 4, expandRect.center().y());
    }
    painter->drawPolygon(triangle);
    
    // Draw group name
    QRect textRect = fullRect;
    textRect.setLeft(expandRect.right() + 8);
    
    QFont font = option.font;
    font.setPointSize(9);
    painter->setFont(font);
    painter->setPen(QPen(QColor(60, 60, 60)));
    
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    
    // Draw subtle separator line at bottom
    painter->setPen(QPen(QColor(230, 230, 230), 1));
    painter->drawLine(fullRect.bottomLeft(), fullRect.bottomRight());
    
    painter->restore();
}

void FileViewDelegate::paintFileItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    
    // Draw selection/hover background with rounded corners
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(QBrush(QColor(0, 120, 215, 60)));
        painter->setPen(QPen(QColor(0, 120, 215), 1));
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 4, 4);
    } else if (option.state & QStyle::State_MouseOver) {
        painter->setBrush(QBrush(QColor(230, 230, 230, 80)));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 4, 4);
    }
    
    // Draw icon
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        QRect iconRect = option.rect;
        iconRect.setSize(QSize(IconSize, IconSize));
        iconRect.moveCenter(QPoint(option.rect.center().x(), option.rect.top() + IconSize/2 + 10));
        
        icon.paint(painter, iconRect);
    }
    
    // Draw text below icon with better formatting
    QRect textRect = option.rect;
    textRect.setTop(textRect.top() + IconSize + 18);
    textRect.setHeight(16);
    textRect.adjust(4, 0, -4, 0); // Add some margin
    
    QFont font = option.font;
    font.setPointSize(8);
    painter->setFont(font);
    painter->setPen(QPen(QColor(50, 50, 50)));
    
    QFontMetrics fm(font);
    QString text = index.data(Qt::DisplayRole).toString();
    QString elidedText = fm.elidedText(text, Qt::ElideRight, textRect.width());
    
    painter->drawText(textRect, Qt::AlignCenter | Qt::AlignTop, elidedText);
    
    painter->restore();
}

QRect FileViewDelegate::getExpandCollapseRect(const QStyleOptionViewItem &option) const
{
    return QRect(option.rect.left() + 8, option.rect.top() + 8, 16, 16);
} 