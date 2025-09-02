#include "fileicondelegate.h"
#include "../models/filesystemmodel.h"
#include "../core/fileitem.h"
#include <QApplication>
#include <QStyle>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QListView>

FileIconDelegate::FileIconDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_viewWidth(800)
{
}

FileIconDelegate::~FileIconDelegate() = default;

void FileIconDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;
        
    bool isGroupHeader = index.data(FileSystemModel::IsGroupHeaderRole).toBool();
    
    if (isGroupHeader) {
        paintGroupHeader(painter, option, index);
    } else {
        paintFileItem(painter, option, index);
    }
}

QSize FileIconDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return QSize();

    bool isGroupHeader = index.data(FileSystemModel::IsGroupHeaderRole).toBool();

    if (isGroupHeader) {
        // Use the most reliable method to get the actual view width
        int width = m_viewWidth; // Default fallback

        // Try to get width from the widget (most reliable)
        if (option.widget) {
            const QListView* listView = qobject_cast<const QListView*>(option.widget);
            if (listView) {
                width = listView->viewport()->width();
            } else {
                width = option.widget->width();
            }
        }

        // Ensure minimum width for group headers
        width = qMax(width, 200);

        return QSize(width, GroupHeaderHeight);
    } else {
        return QSize(FileItemSize, FileItemSize + 20); // Extra space for text
    }
}

bool FileIconDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        
        bool isGroupHeader = index.data(FileSystemModel::IsGroupHeaderRole).toBool();
        if (isGroupHeader) {
            QRect expandRect = getExpandCollapseButtonRect(option);
            if (expandRect.contains(mouseEvent->pos())) {
                FileSystemModel* fsModel = qobject_cast<FileSystemModel*>(model);
                if (fsModel) {
                    QString groupName = index.data(FileSystemModel::GroupNameRole).toString();
                    fsModel->toggleGroupExpansion(groupName);
                    return true;
                }
            }
        }
    }
    
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void FileIconDelegate::paintGroupHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
    QRect expandRect = getExpandCollapseButtonRect(option);
    bool isExpanded = index.data(FileSystemModel::IsExpandedRole).toBool();
    
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
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(QPen(QColor(60, 60, 60)));
    
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    
    // Draw subtle separator line at bottom
    painter->setPen(QPen(QColor(230, 230, 230), 1));
    painter->drawLine(fullRect.bottomLeft(), fullRect.bottomRight());
    
    painter->restore();
}

void FileIconDelegate::paintFileItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
    
    // Get file item data
    FileItem item = index.data(FileSystemModel::FileItemRole).value<FileItem>();
    
    // Draw icon
    if (!item.icon.isNull()) {
        QRect iconRect = option.rect;
        iconRect.setSize(QSize(IconSize, IconSize));
        iconRect.moveCenter(QPoint(option.rect.center().x(), option.rect.top() + IconSize/2 + 10));
        
        item.icon.paint(painter, iconRect);
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
    QString elidedText = fm.elidedText(item.name, Qt::ElideRight, textRect.width());
    
    painter->drawText(textRect, Qt::AlignCenter | Qt::AlignTop, elidedText);
    
    painter->restore();
}

QRect FileIconDelegate::getExpandCollapseButtonRect(const QStyleOptionViewItem& option) const
{
    return QRect(option.rect.left() + 8, option.rect.top() + 8, 16, 16);
}

bool FileIconDelegate::isPointInExpandCollapseButton(const QPoint& point, const QStyleOptionViewItem& option) const
{
    QRect buttonRect = getExpandCollapseButtonRect(option);
    return buttonRect.contains(point);
} 