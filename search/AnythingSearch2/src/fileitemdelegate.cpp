#include "fileitemdelegate.h"
#include "filelistmodel.h"
#include <QPainter>
#include <QApplication>
#include <QFileInfo>
#include <QDateTime>

FileItemDelegate::FileItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    // 绘制背景
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
        painter->setPen(opt.palette.highlightedText().color());
    } else if (opt.state & QStyle::State_MouseOver) {
        painter->fillRect(opt.rect, opt.palette.light());
        painter->setPen(opt.palette.text().color());
    } else {
        painter->setPen(opt.palette.text().color());
    }
    
    // 获取数据
    QString name = index.data(FileListModel::NameRole).toString();
    QString path = index.data(FileListModel::PathRole).toString();
    QString type = index.data(FileListModel::TypeRole).toString();
    qint64 size = index.data(FileListModel::SizeRole).toLongLong();
    QDateTime modified = index.data(FileListModel::ModifiedRole).toDateTime();
    bool isDirectory = index.data(FileListModel::IsDirectoryRole).toBool();
    
    // 绘制图标
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QRect iconRect = opt.rect;
    iconRect.setWidth(iconRect.height());
    
    if (!icon.isNull()) {
        icon.paint(painter, iconRect, Qt::AlignCenter);
    }
    
    // 调整文本区域
    QRect textRect = opt.rect;
    textRect.setLeft(iconRect.right() + 5);
    
    // 绘制文件名
    QFont nameFont = painter->font();
    nameFont.setBold(true);
    painter->setFont(nameFont);
    
    QRect nameRect = textRect;
    nameRect.setHeight(textRect.height() / 2);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);
    
    // 绘制详细信息
    QFont detailFont = painter->font();
    detailFont.setBold(false);
    detailFont.setPointSize(detailFont.pointSize() - 1);
    painter->setFont(detailFont);
    
    QRect detailRect = textRect;
    detailRect.setTop(nameRect.bottom());
    detailRect.setHeight(textRect.height() / 2);
    
    QString details;
    if (isDirectory) {
        details = QString("文件夹 | %1").arg(modified.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        QString sizeStr;
        if (size < 1024) {
            sizeStr = QString("%1 B").arg(size);
        } else if (size < 1024 * 1024) {
            sizeStr = QString("%1 KB").arg(size / 1024.0, 0, 'f', 2);
        } else if (size < 1024 * 1024 * 1024) {
            sizeStr = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2);
        } else {
            sizeStr = QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
        }
        
        details = QString("%1 | %2 | %3").arg(type, sizeStr, modified.toString("yyyy-MM-dd hh:mm:ss"));
    }
    
    painter->drawText(detailRect, Qt::AlignLeft | Qt::AlignVCenter, details);
}

QSize FileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 60);
} 