#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QRect>

class FileSystemModel;

/**
 * @brief Manages complex selection operations for file list view
 * 
 * This class encapsulates all selection logic including cross-group selection,
 * rubber band selection, keyboard modifiers handling, and group-based selection.
 * It follows the Single Responsibility Principle by focusing solely on selection management.
 */
class SelectionManager : public QObject
{
    Q_OBJECT

public:
    enum SelectionMode {
        Single,
        Extended,
        Multi
    };

    explicit SelectionManager(QObject* parent = nullptr);
    ~SelectionManager() override;

    // Core selection operations
    void setModel(FileSystemModel* model);
    void setSelectionModel(QItemSelectionModel* selectionModel);
    
    // Selection manipulation
    void selectIndex(const QModelIndex& index, bool clearPrevious = true);
    void selectIndexes(const QModelIndexList& indexes, bool clearPrevious = true);
    void selectGroup(const QString& groupName);
    void selectRange(const QModelIndex& start, const QModelIndex& end);
    void selectInRect(const QRect& rect, const QAbstractItemView* view);
    void toggleSelection(const QModelIndex& index);
    void clearSelection();
    
    // Selection queries
    QModelIndexList selectedIndexes() const;
    QModelIndexList selectedFileIndexes() const; // Excludes group headers
    bool isSelected(const QModelIndex& index) const;
    int selectedCount() const;
    
    // Modifier key handling
    void handleClick(const QModelIndex& index, Qt::KeyboardModifiers modifiers);
    void handleRangeSelection(const QModelIndex& start, const QModelIndex& end);
    
    // Configuration
    void setSelectionMode(SelectionMode mode);
    SelectionMode selectionMode() const { return m_selectionMode; }

signals:
    void selectionChanged(const QModelIndexList& selected);

private slots:
    void onSelectionModelChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    // Helper methods
    QModelIndexList getGroupFileIndexes(const QString& groupName) const;
    QModelIndexList getIndexesInRange(const QModelIndex& start, const QModelIndex& end) const;
    bool shouldSelectIndex(const QModelIndex& index) const;
    void updateLastSelectedIndex(const QModelIndex& index);

private:
    FileSystemModel* m_model;
    QItemSelectionModel* m_selectionModel;
    SelectionMode m_selectionMode;
    QModelIndex m_lastSelectedIndex;
    QModelIndexList m_currentSelection;
};

#endif // SELECTIONMANAGER_H 