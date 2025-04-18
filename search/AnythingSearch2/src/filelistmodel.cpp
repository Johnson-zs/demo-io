#include "filelistmodel.h"
#include <QFileIconProvider>

FileListModel::FileListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // 添加动态属性以传递关键词
    setProperty("highlightKeyword", "");
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_files.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_files.size())
        return QVariant();
    
    const FileData &fileData = m_files.at(index.row());
    
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return fileData.name;
    case PathRole:
        return fileData.path;
    case TypeRole:
        return fileData.type;
    case SizeRole:
        return fileData.size;
    case ModifiedRole:
        return fileData.modified;
    case IsDirectoryRole:
        return fileData.isDirectory;
    case Qt::DecorationRole: {
        static QFileIconProvider iconProvider;
        QFileInfo fileInfo(fileData.path);
        return iconProvider.icon(fileInfo);
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FileListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[TypeRole] = "type";
    roles[SizeRole] = "size";
    roles[ModifiedRole] = "modified";
    roles[IsDirectoryRole] = "isDirectory";
    return roles;
}

void FileListModel::setFileList(const QVector<FileData> &files)
{
    beginResetModel();
    m_files = files;
    endResetModel();
}

void FileListModel::appendFileList(const QVector<FileData> &files)
{
    if (files.isEmpty())
        return;
    
    beginInsertRows(QModelIndex(), m_files.size(), m_files.size() + files.size() - 1);
    m_files.append(files);
    endInsertRows();
}

void FileListModel::setHighlightKeyword(const QString &keyword)
{
    m_highlightKeyword = keyword;
    setProperty("highlightKeyword", keyword);
    // 通知视图更新
    emit dataChanged(index(0), index(rowCount() - 1));
} 