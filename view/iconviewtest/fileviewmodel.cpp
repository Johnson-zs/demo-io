#include "fileviewmodel.h"
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QPen>

FileViewModel::FileViewModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return visibleItemIndices.size();
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= visibleItemIndices.size())
        return QVariant();
        
    int actualIndex = visibleItemIndices[index.row()];
    const FileViewItem &item = allItems[actualIndex];
    
    switch (role) {
    case Qt::DisplayRole:
        return item.name;
    case Qt::DecorationRole:
        return item.icon;
    case Qt::UserRole: // ItemType
        return static_cast<int>(item.type);
    case Qt::UserRole + 1: // GroupName
        return item.groupName;
    case Qt::UserRole + 2: // IsExpanded
        return item.isExpanded;
    case Qt::UserRole + 3: // GroupId
        return item.groupId;
    default:
        return QVariant();
    }
}

Qt::ItemFlags FileViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
        
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void FileViewModel::addGroup(const QString &groupName)
{
    int groupId = allItems.size();
    allItems.append(FileViewItem(ItemType::GroupHeader, groupName, "", createFolderIcon(), groupId));
    groupExpansionState[groupName] = true;
    rebuildVisibleItems();
}

void FileViewModel::addFileItem(const QString &fileName, const QString &groupName, const QIcon &icon)
{
    // Find the group
    int groupId = -1;
    for (int i = 0; i < allItems.size(); ++i) {
        if (allItems[i].type == ItemType::GroupHeader && allItems[i].name == groupName) {
            groupId = allItems[i].groupId;
            break;
        }
    }
    
    allItems.append(FileViewItem(ItemType::FileItem, fileName, groupName, icon, groupId));
    rebuildVisibleItems();
}

void FileViewModel::toggleGroupExpansion(int index)
{
    if (index < 0 || index >= visibleItemIndices.size())
        return;
        
    int actualIndex = visibleItemIndices[index];
    FileViewItem &item = allItems[actualIndex];
    
    if (item.type == ItemType::GroupHeader) {
        item.isExpanded = !item.isExpanded;
        groupExpansionState[item.name] = item.isExpanded;
        rebuildVisibleItems();
        
        beginResetModel();
        endResetModel();
    }
}

bool FileViewModel::isGroupHeader(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= visibleItemIndices.size())
        return false;
        
    int actualIndex = visibleItemIndices[index.row()];
    return allItems[actualIndex].type == ItemType::GroupHeader;
}

bool FileViewModel::isGroupExpanded(const QString &groupName) const
{
    return groupExpansionState.value(groupName, true);
}

void FileViewModel::rebuildVisibleItems()
{
    visibleItemIndices.clear();
    
    QString currentGroup;
    QList<int> currentGroupItems;
    
    for (int i = 0; i < allItems.size(); ++i) {
        const FileViewItem &item = allItems[i];
        
        if (item.type == ItemType::GroupHeader) {
            // Add any pending group items from previous group
            if (!currentGroupItems.isEmpty() && isGroupExpanded(currentGroup)) {
                visibleItemIndices.append(currentGroupItems);
            }
            
            // Start new group
            currentGroup = item.name;
            currentGroupItems.clear();
            
            // Group headers are always visible
            visibleItemIndices.append(i);
        } else if (item.type == ItemType::FileItem) {
            // Collect file items for the current group
            currentGroupItems.append(i);
        }
    }
    
    // Add remaining items from the last group
    if (!currentGroupItems.isEmpty() && isGroupExpanded(currentGroup)) {
        visibleItemIndices.append(currentGroupItems);
    }
}

QIcon FileViewModel::createFolderIcon()
{
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw folder shadow
    QRect shadowRect(5, 13, 40, 28);
    painter.setBrush(QBrush(QColor(0, 0, 0, 30)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(shadowRect, 2, 2);
    
    // Draw folder back
    QRect folderRect(4, 12, 40, 28);
    QRect tabRect(4, 8, 16, 6);
    
    QColor folderColor(255, 193, 7);
    painter.setBrush(QBrush(folderColor));
    painter.setPen(QPen(folderColor.darker(120), 1));
    
    // Draw tab
    painter.drawRoundedRect(tabRect, 2, 2);
    
    // Draw main folder body
    painter.drawRoundedRect(folderRect, 2, 2);
    
    // Add highlight on folder
    painter.setBrush(QBrush(folderColor.lighter(120)));
    painter.setPen(Qt::NoPen);
    QRect highlightRect(6, 14, 36, 8);
    painter.drawRoundedRect(highlightRect, 1, 1);
    
    return QIcon(pixmap);
}

QIcon FileViewModel::createFileIcon(const QColor &color)
{
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw document icon with shadow
    QRect shadowRect(9, 5, 32, 40);
    painter.setBrush(QBrush(QColor(0, 0, 0, 30)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(shadowRect);
    
    QRect docRect(8, 4, 32, 40);
    QPolygon corner;
    corner << QPoint(32, 4) << QPoint(40, 12) << QPoint(32, 12);
    
    painter.setBrush(QBrush(color));
    painter.setPen(QPen(color.darker(120), 1));
    
    painter.drawRect(docRect);
    painter.drawPolygon(corner);
    
    // Draw fold line
    painter.setPen(QPen(color.darker(140), 1));
    painter.drawLine(32, 12, 40, 12);
    
    // Draw lines to simulate text
    painter.setPen(QPen(color.lighter(180), 2));
    for (int i = 0; i < 4; ++i) {
        int y = 18 + i * 4;
        painter.drawLine(12, y, 28, y);
    }
    
    return QIcon(pixmap);
}

void FileViewModel::populateTestData()
{
    beginResetModel();
    
    allItems.clear();
    visibleItemIndices.clear();
    groupExpansionState.clear();
    
    // Add Microsoft Word group
    addGroup("Microsoft Word 文档 (1)");
    addFileItem("每日工作要点", "Microsoft Word 文档 (1)", createFileIcon(QColor(41, 128, 185)));
    addFileItem("Office文档", "文件夹 (15)", createFileIcon(QColor(231, 76, 60)));

    // Add Text files group  
    addGroup("文本文档 (1)");
    addFileItem("Duplicate Cleaner log", "文本文档 (1)", createFileIcon(QColor(149, 165, 166)));
    
    // Add Folder group with all items from screenshot
    addGroup("文件夹 (15)");
    addFileItem("ABCPhoto", "文件夹 (15)", createFolderIcon());
    addFileItem("Adobe", "文件夹 (15)", createFolderIcon());
    addFileItem("FastViewCloudService", "文件夹 (15)", createFolderIcon());
    addFileItem("My Games", "文件夹 (15)", createFolderIcon());
    addFileItem("My WangWang", "文件夹 (15)", createFolderIcon());
    addFileItem("NetSarang Computer", "文件夹 (15)", createFolderIcon());
    addFileItem("Paradox Interactive", "文件夹 (15)", createFolderIcon());
    addFileItem("Steam", "文件夹 (15)", createFolderIcon());
    addFileItem("Unity", "文件夹 (15)", createFolderIcon());
    addFileItem("Visual Studio", "文件夹 (15)", createFolderIcon());
    addFileItem("Temp Files", "文件夹 (15)", createFolderIcon());
    addFileItem("Downloads", "文件夹 (15)", createFolderIcon());
    addFileItem("Documents", "文件夹 (15)", createFolderIcon());
    addFileItem("Pictures", "文件夹 (15)", createFolderIcon());
    
    endResetModel();
} 