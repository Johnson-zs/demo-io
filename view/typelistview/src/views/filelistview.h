#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QListView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWidget>

class FileSystemModel;
class ContextMenuController;

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
 * @brief Custom QListView for displaying files with grouping support
 * 
 * This view handles file display with custom delegate rendering and context menu support.
 * It includes a header view for column headers and sorting.
 */
class FileListView : public QWidget
{
    Q_OBJECT

public:
    explicit FileListView(QWidget* parent = nullptr);
    ~FileListView() override;

    void setModel(FileSystemModel* model);
    void setContextMenuController(ContextMenuController* controller);
    
    // Delegate some QListView methods
    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex& index);

signals:
    void directoryActivated(const QString& path);
    void sortingChanged(const QString& sortingType, bool ascending);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onItemDoubleClicked(const QModelIndex& index);
    void onHeaderSectionClicked(int logicalIndex);

private:
    void setupView();
    void setupConnections();

private:
    QVBoxLayout* m_layout;
    FileListHeaderView* m_headerView;
    QListView* m_listView;
    FileSystemModel* m_model;
    ContextMenuController* m_contextMenuController;
    Qt::SortOrder m_currentSortOrder;
    int m_currentSortColumn;
};

#endif // FILELISTVIEW_H 