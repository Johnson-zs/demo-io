#include "contextmenucontroller.h"
#include "../models/filesystemmodel.h"
#include "../core/fileitem.h"
#include "../strategies/sortingstrategy.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QActionGroup>

ContextMenuController::ContextMenuController(QObject* parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_emptyAreaMenu(nullptr)
    , m_fileItemMenu(nullptr)
    , m_refreshAction(nullptr)
    , m_openAction(nullptr)
    , m_propertiesAction(nullptr)
    , m_groupingMenu(nullptr)
    , m_sortingMenu(nullptr)
    , m_groupingActionGroup(nullptr)
    , m_sortingActionGroup(nullptr)
    , m_currentSortingType("name")
    , m_currentSortingAscending(true)
{
    createActions();
}

ContextMenuController::~ContextMenuController() = default;

void ContextMenuController::setModel(FileSystemModel* model) {
    m_model = model;
    updateGroupingActions();
}

void ContextMenuController::updateCurrentSortingState(const QString& sortingType, bool ascending) {
    m_currentSortingType = sortingType;
    m_currentSortingAscending = ascending;
    updateSortingActions();
}

void ContextMenuController::showContextMenu(const QPoint& globalPos, const QModelIndex& index) {
    if (!m_model) {
        return;
    }
    
    if (index.isValid() && !m_model->isGroupHeader(index)) {
        // Show file/directory context menu
        showFileItemMenu(globalPos, index);
    } else {
        // Show empty area context menu
        showEmptyAreaMenu(globalPos);
    }
}

void ContextMenuController::onRefreshTriggered() {
    if (m_model) {
        m_model->refreshData();
        emit refreshRequested();
    }
}

void ContextMenuController::onOpenTriggered() {
    // This will be handled by the view's double-click mechanism
    // Just emit a signal for consistency
    emit openRequested();
}

void ContextMenuController::onPropertiesTriggered() {
    // Show a simple properties dialog
    QMessageBox::information(nullptr, tr("Properties"), 
                           tr("File properties dialog would be shown here."));
}

void ContextMenuController::onGroupingTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action || !m_model) {
        return;
    }
    
    QString groupingType = action->data().toString();
    
    std::unique_ptr<GroupingStrategy> strategy;
    
    if (groupingType == "none") {
        strategy = std::make_unique<NoGroupingStrategy>();
    } else if (groupingType == "type") {
        strategy = std::make_unique<TypeGroupingStrategy>();
    } else if (groupingType == "modification_time") {
        strategy = std::make_unique<TimeGroupingStrategy>(TimeGroupingStrategy::ModificationTime);
    } else if (groupingType == "creation_time") {
        strategy = std::make_unique<TimeGroupingStrategy>(TimeGroupingStrategy::CreationTime);
    } else if (groupingType == "name") {
        strategy = std::make_unique<NameGroupingStrategy>();
    } else if (groupingType == "size") {
        strategy = std::make_unique<SizeGroupingStrategy>();
    }
    
    if (strategy) {
        m_model->setGroupingStrategy(std::move(strategy));
        emit groupingChanged(groupingType);
    }
}

void ContextMenuController::onSortingTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action || !m_model) {
        return;
    }
    
    QString sortingData = action->data().toString();
    QStringList parts = sortingData.split("_");
    if (parts.size() != 2) {
        return;
    }
    
    QString sortingType = parts[0];
    bool ascending = (parts[1] == "asc");
    
    m_currentSortingType = sortingType;
    m_currentSortingAscending = ascending;
    
    std::unique_ptr<SortingStrategy> strategy;
    SortingStrategy::SortOrder order = ascending ? SortingStrategy::SortOrder::Ascending : SortingStrategy::SortOrder::Descending;
    
    if (sortingType == "name") {
        strategy = std::make_unique<NameSortingStrategy>();
    } else if (sortingType == "modified") {
        strategy = std::make_unique<ModifiedTimeSortingStrategy>();
    } else if (sortingType == "created") {
        strategy = std::make_unique<CreatedTimeSortingStrategy>();
    } else if (sortingType == "size") {
        strategy = std::make_unique<SizeSortingStrategy>();
    } else if (sortingType == "type") {
        strategy = std::make_unique<TypeSortingStrategy>();
    }
    
    if (strategy) {
        m_model->setSortingStrategy(std::move(strategy), order);
        emit sortingChanged(sortingType, ascending);
    }
}

void ContextMenuController::createActions() {
    // Refresh action
    m_refreshAction = new QAction(this);
    m_refreshAction->setText(tr("Refresh"));
    m_refreshAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &ContextMenuController::onRefreshTriggered);
    
    // Open action
    m_openAction = new QAction(this);
    m_openAction->setText(tr("Open"));
    m_openAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_openAction->setShortcut(QKeySequence(Qt::Key_Return));
    connect(m_openAction, &QAction::triggered, this, &ContextMenuController::onOpenTriggered);
    
    // Properties action
    m_propertiesAction = new QAction(this);
    m_propertiesAction->setText(tr("Properties"));
    m_propertiesAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_propertiesAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Return));
    connect(m_propertiesAction, &QAction::triggered, this, &ContextMenuController::onPropertiesTriggered);
    
    // Create grouping and sorting menus
    createGroupingMenu();
    createSortingMenu();
}

void ContextMenuController::createGroupingMenu() {
    m_groupingMenu = new QMenu(tr("Group By"));
    m_groupingActionGroup = new QActionGroup(this);
    m_groupingActionGroup->setExclusive(true);
    
    // None grouping
    QAction* noneAction = new QAction(tr("None"), this);
    noneAction->setCheckable(true);
    noneAction->setChecked(true); // Default
    noneAction->setData("none");
    m_groupingActionGroup->addAction(noneAction);
    m_groupingMenu->addAction(noneAction);
    connect(noneAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
    
    m_groupingMenu->addSeparator();
    
    // Type grouping
    QAction* typeAction = new QAction(tr("Type"), this);
    typeAction->setCheckable(true);
    typeAction->setData("type");
    m_groupingActionGroup->addAction(typeAction);
    m_groupingMenu->addAction(typeAction);
    connect(typeAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
    
    // Time groupings
    QAction* modTimeAction = new QAction(tr("Modification Time"), this);
    modTimeAction->setCheckable(true);
    modTimeAction->setData("modification_time");
    m_groupingActionGroup->addAction(modTimeAction);
    m_groupingMenu->addAction(modTimeAction);
    connect(modTimeAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
    
    QAction* createTimeAction = new QAction(tr("Creation Time"), this);
    createTimeAction->setCheckable(true);
    createTimeAction->setData("creation_time");
    m_groupingActionGroup->addAction(createTimeAction);
    m_groupingMenu->addAction(createTimeAction);
    connect(createTimeAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
    
    // Name grouping
    QAction* nameAction = new QAction(tr("Name"), this);
    nameAction->setCheckable(true);
    nameAction->setData("name");
    m_groupingActionGroup->addAction(nameAction);
    m_groupingMenu->addAction(nameAction);
    connect(nameAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
    
    // Size grouping
    QAction* sizeAction = new QAction(tr("Size"), this);
    sizeAction->setCheckable(true);
    sizeAction->setData("size");
    m_groupingActionGroup->addAction(sizeAction);
    m_groupingMenu->addAction(sizeAction);
    connect(sizeAction, &QAction::triggered, this, &ContextMenuController::onGroupingTriggered);
}

void ContextMenuController::createSortingMenu() {
    m_sortingMenu = new QMenu(tr("Sort By"));
    m_sortingActionGroup = new QActionGroup(this);
    m_sortingActionGroup->setExclusive(true);
    
    // Name sorting
    QAction* nameAscAction = new QAction(tr("Name (A-Z)"), this);
    nameAscAction->setCheckable(true);
    nameAscAction->setData("name_asc");
    nameAscAction->setChecked(true); // Default
    m_sortingActionGroup->addAction(nameAscAction);
    m_sortingMenu->addAction(nameAscAction);
    connect(nameAscAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    QAction* nameDescAction = new QAction(tr("Name (Z-A)"), this);
    nameDescAction->setCheckable(true);
    nameDescAction->setData("name_desc");
    m_sortingActionGroup->addAction(nameDescAction);
    m_sortingMenu->addAction(nameDescAction);
    connect(nameDescAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    m_sortingMenu->addSeparator();
    
    // Date Modified sorting
    QAction* modifiedAscAction = new QAction(tr("Date Modified (Oldest First)"), this);
    modifiedAscAction->setCheckable(true);
    modifiedAscAction->setData("modified_asc");
    m_sortingActionGroup->addAction(modifiedAscAction);
    m_sortingMenu->addAction(modifiedAscAction);
    connect(modifiedAscAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    QAction* modifiedDescAction = new QAction(tr("Date Modified (Newest First)"), this);
    modifiedDescAction->setCheckable(true);
    modifiedDescAction->setData("modified_desc");
    m_sortingActionGroup->addAction(modifiedDescAction);
    m_sortingMenu->addAction(modifiedDescAction);
    connect(modifiedDescAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    // Date Created sorting
    QAction* createdAscAction = new QAction(tr("Date Created (Oldest First)"), this);
    createdAscAction->setCheckable(true);
    createdAscAction->setData("created_asc");
    m_sortingActionGroup->addAction(createdAscAction);
    m_sortingMenu->addAction(createdAscAction);
    connect(createdAscAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    QAction* createdDescAction = new QAction(tr("Date Created (Newest First)"), this);
    createdDescAction->setCheckable(true);
    createdDescAction->setData("created_desc");
    m_sortingActionGroup->addAction(createdDescAction);
    m_sortingMenu->addAction(createdDescAction);
    connect(createdDescAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    m_sortingMenu->addSeparator();
    
    // Size sorting
    QAction* sizeAscAction = new QAction(tr("Size (Smallest First)"), this);
    sizeAscAction->setCheckable(true);
    sizeAscAction->setData("size_asc");
    m_sortingActionGroup->addAction(sizeAscAction);
    m_sortingMenu->addAction(sizeAscAction);
    connect(sizeAscAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    QAction* sizeDescAction = new QAction(tr("Size (Largest First)"), this);
    sizeDescAction->setCheckable(true);
    sizeDescAction->setData("size_desc");
    m_sortingActionGroup->addAction(sizeDescAction);
    m_sortingMenu->addAction(sizeDescAction);
    connect(sizeDescAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    // Type sorting
    QAction* typeAscAction = new QAction(tr("Type (A-Z)"), this);
    typeAscAction->setCheckable(true);
    typeAscAction->setData("type_asc");
    m_sortingActionGroup->addAction(typeAscAction);
    m_sortingMenu->addAction(typeAscAction);
    connect(typeAscAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
    
    QAction* typeDescAction = new QAction(tr("Type (Z-A)"), this);
    typeDescAction->setCheckable(true);
    typeDescAction->setData("type_desc");
    m_sortingActionGroup->addAction(typeDescAction);
    m_sortingMenu->addAction(typeDescAction);
    connect(typeDescAction, &QAction::triggered, this, &ContextMenuController::onSortingTriggered);
}

void ContextMenuController::showEmptyAreaMenu(const QPoint& globalPos) {
    if (!m_emptyAreaMenu) {
        m_emptyAreaMenu = new QMenu();
        m_emptyAreaMenu->addMenu(m_groupingMenu);
        m_emptyAreaMenu->addMenu(m_sortingMenu);
        m_emptyAreaMenu->addSeparator();
        m_emptyAreaMenu->addAction(m_refreshAction);
    }
    
    updateGroupingActions();
    updateSortingActions();
    m_emptyAreaMenu->exec(globalPos);
}

void ContextMenuController::showFileItemMenu(const QPoint& globalPos, const QModelIndex& index) {
    if (!m_fileItemMenu) {
        m_fileItemMenu = new QMenu();
        m_fileItemMenu->addAction(m_openAction);
        m_fileItemMenu->addSeparator();
        m_fileItemMenu->addAction(m_propertiesAction);
    }
    
    // Update actions based on the selected item
    FileItem item = m_model->getFileItem(index);
    if (item.isDirectory) {
        m_openAction->setText(tr("Open"));
        m_openAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
    } else {
        m_openAction->setText(tr("Open"));
        m_openAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    }
    
    m_fileItemMenu->exec(globalPos);
}

void ContextMenuController::updateGroupingActions() {
    if (!m_model || !m_groupingActionGroup) {
        return;
    }
    
    // Get current grouping strategy name
    QString currentStrategy = "none"; // Default
    if (m_model->groupingStrategy()) {
        QString strategyName = m_model->groupingStrategy()->getStrategyName().toLower();
        
        if (strategyName == "type") {
            currentStrategy = "type";
        } else if (strategyName == "modification time") {
            currentStrategy = "modification_time";
        } else if (strategyName == "creation time") {
            currentStrategy = "creation_time";
        } else if (strategyName == "name") {
            currentStrategy = "name";
        } else if (strategyName == "size") {
            currentStrategy = "size";
        }
    }
    
    // Update checked state
    for (QAction* action : m_groupingActionGroup->actions()) {
        QString actionData = action->data().toString();
        action->setChecked(actionData == currentStrategy);
    }
} 

void ContextMenuController::updateSortingActions() {
    if (!m_model || !m_sortingActionGroup) {
        return;
    }
    
    // Construct the current sorting data string
    QString currentSortingData = m_currentSortingType + "_" + (m_currentSortingAscending ? "asc" : "desc");
    
    // Update checked state
    for (QAction* action : m_sortingActionGroup->actions()) {
        QString actionData = action->data().toString();
        action->setChecked(actionData == currentSortingData);
    }
} 