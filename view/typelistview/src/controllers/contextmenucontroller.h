#ifndef CONTEXTMENUCONTROLLER_H
#define CONTEXTMENUCONTROLLER_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QModelIndex>

class FileSystemModel;

/**
 * @brief Controller for managing context menus in the file browser
 * 
 * This class handles the creation and management of context menus for both
 * empty areas and file items, including grouping options and file operations.
 */
class ContextMenuController : public QObject
{
    Q_OBJECT

public:
    explicit ContextMenuController(QObject* parent = nullptr);
    ~ContextMenuController() override;

    void setModel(FileSystemModel* model);
    void showContextMenu(const QPoint& globalPos, const QModelIndex& index);
    void updateCurrentSortingState(const QString& sortingType, bool ascending);
    void updateCurrentGroupingState(const QString& groupingType, bool ascending);

signals:
    void refreshRequested();
    void openRequested();
    void groupingChanged(const QString& groupingType);
    void groupOrderChanged(const QString& groupingType, bool ascending);
    void sortingChanged(const QString& sortingType, bool ascending);

private slots:
    void onRefreshTriggered();
    void onOpenTriggered();
    void onPropertiesTriggered();
    void onGroupingTriggered();
    void onSortingTriggered();

private:
    void createActions();
    void createGroupingMenu();
    void createSortingMenu();
    void showEmptyAreaMenu(const QPoint& globalPos);
    void showFileItemMenu(const QPoint& globalPos, const QModelIndex& index);
    void updateGroupingActions();
    void updateSortingActions();

private:
    FileSystemModel* m_model;
    
    // Menus
    QMenu* m_emptyAreaMenu;
    QMenu* m_fileItemMenu;
    QMenu* m_groupingMenu;
    QMenu* m_sortingMenu;
    
    // Actions
    QAction* m_refreshAction;
    QAction* m_openAction;
    QAction* m_propertiesAction;
    
    // Grouping actions
    QActionGroup* m_groupingActionGroup;
    
    // Sorting actions
    QActionGroup* m_sortingActionGroup;
    QString m_currentSortingType;
    bool m_currentSortingAscending;
    
    // Grouping state
    QString m_currentGroupingType;
    bool m_currentGroupingAscending;
};

#endif // CONTEXTMENUCONTROLLER_H 