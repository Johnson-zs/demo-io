#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QListView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRubberBand>
#include <QItemSelectionModel>

class FileSystemModel;
class ContextMenuController;
class SelectionManager;
class FileItemDelegate;
class FileIconDelegate;

/**
 * @brief View mode enumeration for FileListView
 */
enum class ViewMode {
    ListView,
    IconView
};

/**
 * @brief Custom header view for the file list
 * 
 * This header view provides column headers for the file list with sorting capabilities.
 */
class FileListHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit FileListHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;

private:
    void setupHeader();
};

/**
 * @brief Custom QListView for displaying files with grouping support and multi-selection
 * 
 * This view handles file display with custom delegate rendering, context menu support,
 * and advanced multi-selection capabilities including cross-group selection.
 * It supports both list view and icon view modes with the same underlying QListView.
 */
class FileListView : public QWidget
{
    Q_OBJECT

public:
    explicit FileListView(QWidget* parent = nullptr);
    ~FileListView() override;

    void setModel(FileSystemModel* model);
    void setContextMenuController(ContextMenuController* controller);
    
    // View mode management
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const { return m_viewMode; }
    
    // Selection methods
    QModelIndexList selectedIndexes() const;
    void selectGroup(const QString& groupName);
    void clearSelection();
    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    
    // Delegate some QListView methods
    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& index);

signals:
    void directoryActivated(const QString& path);
    void sortingChanged(const QString& sortingType, bool ascending);
    void selectionChanged(const QModelIndexList& selected);
    void viewModeChanged(ViewMode mode);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onItemDoubleClicked(const QModelIndex& index);
    void onHeaderSectionClicked(int logicalIndex);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onGroupHeaderClicked(const QModelIndex& index);

private:
    void setupView();
    void setupConnections();
    void updateRubberBandSelection();
    QModelIndexList getIndexesInRect(const QRect& rect) const;
    bool isGroupHeader(const QModelIndex& index) const;
    void handleGroupHeaderClick(const QModelIndex& index);
    void switchToListMode();
    void switchToIconMode();

private:
    QVBoxLayout* m_layout;
    FileListHeaderView* m_headerView;
    QListView* m_listView;
    FileSystemModel* m_model;
    ContextMenuController* m_contextMenuController;
    SelectionManager* m_selectionManager;
    Qt::SortOrder m_currentSortOrder;
    int m_currentSortColumn;
    
    // View mode and delegates
    ViewMode m_viewMode;
    FileItemDelegate* m_listDelegate;
    FileIconDelegate* m_iconDelegate;
    
    // Multi-selection support
    QRubberBand* m_rubberBand;
    QPoint m_rubberBandOrigin;
    bool m_rubberBandActive;
    QModelIndex m_lastClickedIndex;
};

#endif // FILELISTVIEW_H 