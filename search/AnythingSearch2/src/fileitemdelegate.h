#ifndef FILEITEMDELEGATE_H
#define FILEITEMDELEGATE_H

#include <QStyledItemDelegate>

class FileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FileItemDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString highlightText(const QString &text, const QString &highlight) const;
    QString highlightSingleKeyword(const QString &htmlText, const QString &keyword) const;
    QString extractTextFromHtml(const QString &html) const;
    int findHtmlIndex(const QString &html, int textIndex) const;
};

#endif // FILEITEMDELEGATE_H 