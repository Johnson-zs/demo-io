#include "mainwindow.h"
#include "src/models/filesystemmodel.h"
#include "src/views/filelistview.h"
#include "src/controllers/contextmenucontroller.h"
#include "src/strategies/sortingstrategy.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStyle>
#include <QLabel>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_model(nullptr)
    , m_listView(nullptr)
    , m_contextMenuController(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_toolBar(nullptr)
    , m_sortingCombo(nullptr)
    , m_groupingCombo(nullptr)
    , m_refreshAction(nullptr)
    , m_homeAction(nullptr)
    , m_upAction(nullptr)
    , m_pathLabel(nullptr)
    , m_itemCountLabel(nullptr)
    , m_loadingLabel(nullptr)
    , m_pathEdit(nullptr)
{
    setupUI();
    setupToolBar();
    setupStatusBar();
    setupConnections();
    
    // Initialize with home directory
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    navigateToPath(homePath);
}

MainWindow::~MainWindow() = default;

void MainWindow::onDirectoryChanged(const QString& path) {
    m_currentPath = path;
    updatePathEdit(path);
    updateStatusBar();
}

void MainWindow::updatePathEdit(const QString& path) {
    if (m_pathEdit && m_pathEdit->text() != path) {
        m_pathEdit->setText(path);
    }
}

void MainWindow::onLoadingStarted() {
    if (m_loadingLabel) {
        m_loadingLabel->setText(tr("Loading..."));
        m_loadingLabel->show();
    }
}

void MainWindow::onLoadingFinished() {
    if (m_loadingLabel) {
        m_loadingLabel->hide();
    }
    updateStatusBar();
}

void MainWindow::onSortingChanged() {
    if (!m_model || !m_sortingCombo) {
        return;
    }
    
    QString sortingData = m_sortingCombo->currentData().toString();
    QStringList parts = sortingData.split("_");
    if (parts.size() != 2) {
        return;
    }
    
    QString sortingType = parts[0];
    bool ascending = (parts[1] == "asc");
    
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
    }
}

void MainWindow::onGroupingChanged() {
    if (!m_model || !m_groupingCombo) {
        return;
    }
    
    QString groupingData = m_groupingCombo->currentData().toString();
    
    QString groupingType;
    bool ascending = true;
    
    if (groupingData == "none") {
        groupingType = "none";
    } else if (groupingData.endsWith("_asc") || groupingData.endsWith("_desc")) {
        ascending = groupingData.endsWith("_asc");
        groupingType = groupingData.left(groupingData.lastIndexOf("_"));
    } else {
        return; // Invalid format
    }
    
    std::unique_ptr<GroupingStrategy> strategy;
    GroupingStrategy::GroupOrder order = ascending ? GroupingStrategy::GroupOrder::Ascending : GroupingStrategy::GroupOrder::Descending;
    
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
        if (groupingType == "none") {
            m_model->setGroupingStrategy(std::move(strategy));
        } else {
            m_model->setGroupingStrategy(std::move(strategy), order);
        }
        
        // Update context menu controller state
        m_contextMenuController->updateCurrentGroupingState(groupingType, ascending);
    }
}

void MainWindow::setupUI() {
    // Create central widget and layout
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create the model
    m_model = new FileSystemModel(this);
    
    // Create the list view (now a composite widget with header)
    m_listView = new FileListView(this);
    m_listView->setModel(m_model);
    
    // Create context menu controller
    m_contextMenuController = new ContextMenuController(this);
    m_contextMenuController->setModel(m_model);
    m_listView->setContextMenuController(m_contextMenuController);
    
    // Add list view to layout
    m_mainLayout->addWidget(m_listView);
    
    // Set window properties
    setWindowTitle(tr("File Browser"));
    setMinimumSize(800, 600);
    resize(1200, 700);
}

void MainWindow::setupToolBar() {
    m_toolBar = addToolBar(tr("Main"));
    m_toolBar->setMovable(false);
    
    // Home action
    m_homeAction = new QAction(this);
    m_homeAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon));
    m_homeAction->setText(tr("Home"));
    m_homeAction->setToolTip(tr("Go to home directory"));
    m_toolBar->addAction(m_homeAction);
    
    // Up directory action
    m_upAction = new QAction(this);
    m_upAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogToParent));
    m_upAction->setText(tr("Up"));
    m_upAction->setToolTip(tr("Go to parent directory"));
    m_toolBar->addAction(m_upAction);
    
    // Refresh action
    m_refreshAction = new QAction(this);
    m_refreshAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    m_refreshAction->setText(tr("Refresh"));
    m_refreshAction->setToolTip(tr("Refresh current directory"));
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    m_toolBar->addAction(m_refreshAction);
    
    m_toolBar->addSeparator();
    
    // Path edit
    m_toolBar->addWidget(new QLabel(tr("Path:")));
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setMinimumWidth(300);
    m_pathEdit->setPlaceholderText(tr("Enter path and press Enter"));
    m_pathEdit->setToolTip(tr("Enter a directory path to navigate to"));
    m_toolBar->addWidget(m_pathEdit);
    
    m_toolBar->addSeparator();
    
    // Sorting combo
    m_toolBar->addWidget(new QLabel(tr("Sort:")));
    m_sortingCombo = new QComboBox(this);
    m_sortingCombo->addItem(tr("Name (A-Z)"), "name_asc");
    m_sortingCombo->addItem(tr("Name (Z-A)"), "name_desc");
    m_sortingCombo->insertSeparator(2);
    m_sortingCombo->addItem(tr("Date Modified (Oldest First)"), "modified_asc");
    m_sortingCombo->addItem(tr("Date Modified (Newest First)"), "modified_desc");
    m_sortingCombo->addItem(tr("Date Created (Oldest First)"), "created_asc");
    m_sortingCombo->addItem(tr("Date Created (Newest First)"), "created_desc");
    m_sortingCombo->insertSeparator(7);
    m_sortingCombo->addItem(tr("Size (Smallest First)"), "size_asc");
    m_sortingCombo->addItem(tr("Size (Largest First)"), "size_desc");
    m_sortingCombo->addItem(tr("Type (A-Z)"), "type_asc");
    m_sortingCombo->addItem(tr("Type (Z-A)"), "type_desc");
    m_sortingCombo->setCurrentIndex(0);
    m_toolBar->addWidget(m_sortingCombo);
    
    m_toolBar->addSeparator();
    
    // Grouping combo
    m_toolBar->addWidget(new QLabel(tr("Group:")));
    m_groupingCombo = new QComboBox(this);
    m_groupingCombo->addItem(tr("None"), "none");
    m_groupingCombo->insertSeparator(1);
    
    // Type grouping
    m_groupingCombo->addItem(tr("Type (Ascending)"), "type_asc");
    m_groupingCombo->addItem(tr("Type (Descending)"), "type_desc");
    m_groupingCombo->insertSeparator(4);
    
    // Time grouping
    m_groupingCombo->addItem(tr("Modification Time (Today → Earlier)"), "modification_time_asc");
    m_groupingCombo->addItem(tr("Modification Time (Earlier → Today)"), "modification_time_desc");
    m_groupingCombo->addItem(tr("Creation Time (Today → Earlier)"), "creation_time_asc");
    m_groupingCombo->addItem(tr("Creation Time (Earlier → Today)"), "creation_time_desc");
    m_groupingCombo->insertSeparator(9);
    
    // Name grouping
    m_groupingCombo->addItem(tr("Name (0-9 → Other)"), "name_asc");
    m_groupingCombo->addItem(tr("Name (Other → 0-9)"), "name_desc");
    
    // Size grouping
    m_groupingCombo->addItem(tr("Size (Unknown → Massive)"), "size_asc");
    m_groupingCombo->addItem(tr("Size (Massive → Unknown)"), "size_desc");
    
    m_groupingCombo->setCurrentIndex(0);
    m_toolBar->addWidget(m_groupingCombo);
    
    // Connect toolbar actions
    connect(m_homeAction, &QAction::triggered, this, [this]() {
        QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        navigateToPath(homePath);
    });
    
    connect(m_upAction, &QAction::triggered, this, &MainWindow::onUpDirectoryTriggered);
    
    connect(m_refreshAction, &QAction::triggered, this, [this]() {
        if (m_model) {
            m_model->refreshData();
        }
    });
    
    connect(m_pathEdit, &QLineEdit::returnPressed, this, &MainWindow::onPathEditingFinished);
    
    connect(m_sortingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortingChanged);
    
    connect(m_groupingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onGroupingChanged);
}

void MainWindow::setupStatusBar() {
    // Path label
    m_pathLabel = new QLabel(this);
    m_pathLabel->setMinimumWidth(200);
    statusBar()->addWidget(m_pathLabel);
    
    statusBar()->addPermanentWidget(new QLabel("|"));
    
    // Item count label
    m_itemCountLabel = new QLabel(this);
    m_itemCountLabel->setMinimumWidth(100);
    statusBar()->addPermanentWidget(m_itemCountLabel);
    
    statusBar()->addPermanentWidget(new QLabel("|"));
    
    // Loading label
    m_loadingLabel = new QLabel(this);
    m_loadingLabel->hide();
    statusBar()->addPermanentWidget(m_loadingLabel);
}

void MainWindow::setupConnections() {
    // Model connections
    connect(m_model, &FileSystemModel::directoryLoaded,
            this, &MainWindow::onDirectoryChanged);
    connect(m_model, &FileSystemModel::loadingStarted,
            this, &MainWindow::onLoadingStarted);
    connect(m_model, &FileSystemModel::loadingFinished,
            this, &MainWindow::onLoadingFinished);
    
    // View connections
    connect(m_listView, &FileListView::directoryActivated,
            this, &MainWindow::navigateToPath);
    
    connect(m_listView, &FileListView::sortingChanged,
            this, [this](const QString& sortingType, bool ascending) {
                // Update combo box to match header click
                QString sortingData = sortingType + "_" + (ascending ? "asc" : "desc");
                int index = m_sortingCombo->findData(sortingData);
                if (index >= 0) {
                    m_sortingCombo->setCurrentIndex(index);
                }
                
                // Update context menu controller's current sorting state
                if (m_contextMenuController) {
                    m_contextMenuController->updateCurrentSortingState(sortingType, ascending);
                }
            });
    
    // Selection change connection
    connect(m_listView, &FileListView::selectionChanged,
            this, [this](const QModelIndexList& selected) {
                updateStatusBar(); // Update status bar to show selection count
            });
    
    // Context menu connections
    connect(m_contextMenuController, &ContextMenuController::refreshRequested,
            this, [this]() {
                if (m_model) {
                    m_model->refreshData();
                }
            });
    
    connect(m_contextMenuController, &ContextMenuController::groupOrderChanged,
            this, [this](const QString& groupingType, bool ascending) {
                // Update combo box to match context menu selection
                QString groupingData;
                if (groupingType == "none") {
                    groupingData = "none";
                } else {
                    groupingData = groupingType + "_" + (ascending ? "asc" : "desc");
                }
                
                int index = m_groupingCombo->findData(groupingData);
                if (index >= 0) {
                    // Temporarily disconnect to avoid recursive calls
                    m_groupingCombo->blockSignals(true);
                    m_groupingCombo->setCurrentIndex(index);
                    m_groupingCombo->blockSignals(false);
                }
            });
    
    connect(m_contextMenuController, &ContextMenuController::groupOrderChanged,
            this, [this](const QString& groupingType, bool ascending) {
                // Update the context menu controller's internal state
                m_contextMenuController->updateCurrentGroupingState(groupingType, ascending);
            });
    
    connect(m_contextMenuController, &ContextMenuController::sortingChanged,
            this, [this](const QString& sortingType, bool ascending) {
                // Update combo box to match context menu selection
                QString sortingData = sortingType + "_" + (ascending ? "asc" : "desc");
                int index = m_sortingCombo->findData(sortingData);
                if (index >= 0) {
                    m_sortingCombo->setCurrentIndex(index);
                }
            });
}

void MainWindow::updateStatusBar() {
    if (!m_model) {
        return;
    }
    
    // Update path
    if (m_pathLabel) {
        QString displayPath = m_currentPath;
        if (displayPath.length() > 50) {
            displayPath = "..." + displayPath.right(47);
        }
        m_pathLabel->setText(displayPath);
        m_pathLabel->setToolTip(m_currentPath);
    }
    
    // Update item count and selection info
    if (m_itemCountLabel) {
        int totalItems = 0;
        int visibleItems = m_model->rowCount();
        
        // Count total items (including those in collapsed groups)
        for (int i = 0; i < m_model->rowCount(); ++i) {
            QModelIndex index = m_model->index(i, 0);
            if (m_model->isGroupHeader(index)) {
                // This is a group header, count items in the group
                // For now, we'll just use visible items count
            } else {
                totalItems++;
            }
        }
        
        // Get selection count
        int selectedCount = m_listView ? m_listView->selectedIndexes().count() : 0;
        
        QString statusText;
        if (selectedCount > 0) {
            if (totalItems != visibleItems) {
                statusText = tr("%1 items (%2 visible), %3 selected")
                    .arg(totalItems).arg(visibleItems).arg(selectedCount);
            } else {
                statusText = tr("%1 items, %2 selected").arg(totalItems).arg(selectedCount);
            }
        } else {
            if (totalItems != visibleItems) {
                statusText = tr("%1 items (%2 visible)").arg(totalItems).arg(visibleItems);
            } else {
                statusText = tr("%1 items").arg(totalItems);
            }
        }
        
        m_itemCountLabel->setText(statusText);
    }
}

void MainWindow::navigateToPath(const QString& path) {
    if (path.isEmpty() || !QDir(path).exists()) {
        QMessageBox::warning(this, tr("Error"), tr("Directory does not exist: %1").arg(path));
        return;
    }
    
    if (m_model) {
        m_model->setRootPath(path);
    }
}

void MainWindow::onPathEditingFinished() {
    if (!m_pathEdit) {
        return;
    }
    
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty()) {
        return;
    }
    
    // Expand ~ to home directory
    if (path.startsWith("~")) {
        path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + path.mid(1);
    }
    
    // Check if directory exists
    if (!QDir(path).exists()) {
        QMessageBox::warning(this, tr("Error"), tr("Directory does not exist: %1").arg(path));
        // Restore the current path
        updatePathEdit(m_currentPath);
        return;
    }
    
    // Navigate to the path
    navigateToPath(path);
}

void MainWindow::onUpDirectoryTriggered() {
    if (m_currentPath.isEmpty()) {
        return;
    }
    
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        navigateToPath(dir.absolutePath());
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Cannot go up from: %1").arg(m_currentPath));
    }
}
