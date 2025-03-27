#include "fileitemdelegate.h"
#include "filelistmodel.h"
#include <QPainter>
#include <QApplication>
#include <QFileInfo>
#include <QDateTime>
#include <QTextDocument>
#include <QRegularExpression>

FileItemDelegate::FileItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 优化绘制逻辑
    if (!index.isValid())
        return;
        
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
    
    // 获取数据 - 使用局部变量缓存，避免多次调用model方法
    QString name = index.data(FileListModel::NameRole).toString();
    QString path = index.data(FileListModel::PathRole).toString();
    QString type = index.data(FileListModel::TypeRole).toString();
    qint64 size = index.data(FileListModel::SizeRole).toLongLong();
    QDateTime modified = index.data(FileListModel::ModifiedRole).toDateTime();
    bool isDirectory = index.data(FileListModel::IsDirectoryRole).toBool();
    
    // 剪裁文本使其适合显示区域，避免不必要的绘制
    QString displayName = painter->fontMetrics().elidedText(name, Qt::ElideMiddle, opt.rect.width() - 60);
    
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
    
    // 获取高亮关键词
    QString keyword = index.model()->property("highlightKeyword").toString();
    
    // 绘制文件名（带高亮）
    QFont nameFont = painter->font();
    nameFont.setBold(true);
    painter->setFont(nameFont);
    
    QRect nameRect = textRect;
    nameRect.setHeight(textRect.height() / 2);
    
    // 绘制普通文本
    if (keyword.isEmpty()) {
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
    } else {
        // 直接绘制高亮文本
        QStringList keywords = keyword.split(' ', Qt::SkipEmptyParts);
        QString nameToDraw = displayName;
        
        // 计算每个关键词在文本中的位置
        QVector<QPair<int, int>> highlightPositions;
        
        for (const QString &kw : keywords) {
            int pos = 0;
            while ((pos = nameToDraw.toLower().indexOf(kw.toLower(), pos)) != -1) {
                highlightPositions.append(qMakePair(pos, kw.length()));
                pos += kw.length();
            }
        }
        
        // 按位置排序
        std::sort(highlightPositions.begin(), highlightPositions.end(), 
                 [](const QPair<int, int> &a, const QPair<int, int> &b) {
                     return a.first < b.first;
                 });
        
        // 绘制文本和高亮
        int currentPos = 0;
        int textX = nameRect.left();
        
        for (const auto &highlight : highlightPositions) {
            // 绘制高亮前的文本
            if (highlight.first > currentPos) {
                QString beforeText = nameToDraw.mid(currentPos, highlight.first - currentPos);
                QRect beforeRect = nameRect;
                beforeRect.setLeft(textX);
                painter->drawText(beforeRect, Qt::AlignLeft | Qt::AlignVCenter, beforeText);
                textX += painter->fontMetrics().horizontalAdvance(beforeText);
            }
            
            // 绘制高亮背景
            QString highlightedText = nameToDraw.mid(highlight.first, highlight.second);
            QRect highlightRect = nameRect;
            highlightRect.setLeft(textX);
            highlightRect.setWidth(painter->fontMetrics().horizontalAdvance(highlightedText));
            painter->fillRect(highlightRect, QColor(255, 255, 0, 128)); // 半透明黄色
            
            // 绘制高亮文本
            painter->drawText(highlightRect, Qt::AlignLeft | Qt::AlignVCenter, highlightedText);
            
            textX += highlightRect.width();
            currentPos = highlight.first + highlight.second;
        }
        
        // 绘制剩余文本
        if (currentPos < nameToDraw.length()) {
            QString afterText = nameToDraw.mid(currentPos);
            QRect afterRect = nameRect;
            afterRect.setLeft(textX);
            painter->drawText(afterRect, Qt::AlignLeft | Qt::AlignVCenter, afterText);
        }
    }
    
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

QString FileItemDelegate::highlightText(const QString &text, const QString &highlight) const
{
    if (highlight.isEmpty()) {
        return text;
    }
    
    // 拆分搜索关键词（按空格分割）
    QStringList keywords = highlight.split(' ', Qt::SkipEmptyParts);
    
    // 创建临时文档用于处理HTML
    QTextDocument doc;
    
    // 使用纯文本作为起点
    QString plainText = text;
    QString htmlText = QString("<span style=\"color: %1;\">%2</span>")
                        .arg(QApplication::palette().text().color().name(), 
                             plainText.toHtmlEscaped());
    
    // 为每个关键词添加高亮
    for (const QString &keyword : keywords) {
        if (keyword.isEmpty()) continue;
        
        // 使用Qt的文本文档处理高亮
        QRegularExpression regex(QRegularExpression::escape(keyword), 
                                QRegularExpression::CaseInsensitiveOption);
        
        htmlText.replace(regex, 
                        QString("<span style=\"background-color: yellow;\">\\0</span>"));
    }
    
    return htmlText;
}

QSize FileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 60);
} 