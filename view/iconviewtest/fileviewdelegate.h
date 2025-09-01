#ifndef FILEVIEWDELEGATE_H
#define FILEVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QApplication>
#include <QStyle>
#include <QFontMetrics>
#include <QMouseEvent>

class FileViewModel;

class FileViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FileViewDelegate(QObject *parent = nullptr);
    
    // QStyledItemDelegate interface
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void paintGroupHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintFileItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
    QRect getExpandCollapseRect(const QStyleOptionViewItem &option) const;
    
    static const int GroupHeaderHeight = 28;
    static const int FileItemSize = 80;
    static const int FileItemSpacing = 10;
    static const int IconSize = 48;

    int m_viewWidth = 800; // Default width, will be updated by ListView

};

#endif // FILEVIEWDELEGATE_H 
