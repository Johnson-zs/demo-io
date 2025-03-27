#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "filedata.h"

class FileListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        TypeRole,
        SizeRole,
        ModifiedRole,
        IsDirectoryRole
    };

    explicit FileListModel(QObject *parent = nullptr);
    
    // 必须实现的QAbstractListModel方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // 更新文件列表
    void setFileList(const QVector<FileData> &files);
    
    // 追加文件列表
    void appendFileList(const QVector<FileData> &files);
    
    // 设置关键词用于高亮
    void setHighlightKeyword(const QString &keyword);

private:
    QVector<FileData> m_files;
    QString m_highlightKeyword;
};

#endif // FILELISTMODEL_H 