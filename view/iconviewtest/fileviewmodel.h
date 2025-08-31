#ifndef FILEVIEWMODEL_H
#define FILEVIEWMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QStyle>

enum class ItemType {
    GroupHeader,
    FileItem
};

struct FileViewItem {
    ItemType type;
    QString name;
    QString groupName;
    QIcon icon;
    bool isExpanded = true; // Only relevant for group headers
    int groupId = -1;
    
    FileViewItem(ItemType t, const QString& n, const QString& group = "", const QIcon& i = QIcon(), int gid = -1)
        : type(t), name(n), groupName(group), icon(i), groupId(gid) {}
};

class FileViewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FileViewModel(QObject *parent = nullptr);
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    // Custom methods
    void addGroup(const QString &groupName);
    void addFileItem(const QString &fileName, const QString &groupName, const QIcon &icon);
    void toggleGroupExpansion(int index);
    bool isGroupHeader(const QModelIndex &index) const;
    bool isGroupExpanded(const QString &groupName) const;
    
    // Test data population
    void populateTestData();

private:
    void rebuildVisibleItems();
    QIcon createFolderIcon();
    QIcon createFileIcon(const QColor &color);
    
    QList<FileViewItem> allItems;
    QList<int> visibleItemIndices; // Indices into allItems that should be visible
    QMap<QString, bool> groupExpansionState;
};

#endif // FILEVIEWMODEL_H 