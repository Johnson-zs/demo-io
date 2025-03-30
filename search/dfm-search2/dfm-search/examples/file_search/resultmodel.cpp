#include "resultmodel.h"

#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>
#include <QDateTime>

ResultModel::ResultModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // 初始化列名
    headerLabels_ << "名称" << "路径" << "大小" << "修改时间";
}

ResultModel::~ResultModel() = default;

QModelIndex ResultModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();
    
    return createIndex(row, column);
}

QModelIndex ResultModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return results_.items().size();
}

int ResultModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return headerLabels_.size();
}

QVariant ResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(results_.items().size()))
        return QVariant();
    
    const auto &item = results_.items()[index.row()];
    
    if (role == Qt::DisplayRole) {
        // 根据列显示不同的数据
        switch (index.column()) {
            case 0: // 名称
                return item->displayName();
                
            case 1: // 路径
                if (auto fileItem = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(item)) {
                    return fileItem->filePath();
                }
                return item->uri().toString();
                
            case 2: // 大小
                if (auto fileItem = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(item)) {
                    if (fileItem->isDirectory()) {
                        return "<目录>";
                    } else {
                        qint64 size = fileItem->fileSize();
                        if (size < 1024) {
                            return QString("%1 B").arg(size);
                        } else if (size < 1024 * 1024) {
                            return QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
                        } else if (size < 1024 * 1024 * 1024) {
                            return QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
                        } else {
                            return QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
                        }
                    }
                }
                return QVariant();
                
            case 3: // 修改时间
                return item->lastModified().toString("yyyy-MM-dd hh:mm:ss");
        }
    } 
    else if (role == Qt::DecorationRole && index.column() == 0) {
        // 第一列显示图标
        if (auto fileItem = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(item)) {
            if (fileItem->isDirectory()) {
                return QIcon::fromTheme("folder");
            } else {
                QMimeDatabase db;
                QFileInfo fileInfo(fileItem->filePath());
                QMimeType mime = db.mimeTypeForFile(fileInfo);
                return QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("text-x-generic"));
            }
        } 
        else if (auto appItem = std::dynamic_pointer_cast<DFM::Search::AppResultItem>(item)) {
            return QIcon::fromTheme(appItem->iconName(), QIcon::fromTheme("application-x-executable"));
        }
    }
    
    return QVariant();
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < headerLabels_.size()) {
        return headerLabels_[section];
    }
    
    return QVariant();
}

void ResultModel::setResults(const DFM::Search::SearchResult &results)
{
    beginResetModel();
    results_ = results;
    endResetModel();
}

void ResultModel::clear()
{
    beginResetModel();
    results_.clear();
    endResetModel();
}

std::shared_ptr<DFM::Search::SearchResultItem> ResultModel::getResultItem(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(results_.items().size()))
        return nullptr;
    
    return results_.items()[index.row()];
} 