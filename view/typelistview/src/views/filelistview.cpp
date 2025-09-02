#include "filelistview.h"
#include "fileitemdelegate.h"
#include "fileicondelegate.h"
#include "selectionmanager.h"
#include "../models/filesystemmodel.h"
#include "../controllers/contextmenucontroller.h"
#include "../core/fileitem.h"
#include "../strategies/sortingstrategy.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QRubberBand>

// FileListHeaderView implementation
FileListHeaderView::FileListHeaderView(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)
{
    setupHeader();
}

void FileListHeaderView::setupHeader() {
    setDefaultSectionSize(150);
    setMinimumSectionSize(80);
    setSectionResizeMode(QHeaderView::Interactive);
    setStretchLastSection(true);
    setSortIndicatorShown(true);
    setSectionsClickable(true);
    
    // Set a reasonable fixed height for the header
    setFixedHeight(24);
    setMinimumHeight(24);
    setMaximumHeight(24);
    
    // Set initial section sizes
    resizeSection(0, 250); // Name column
    resizeSection(1, 150); // Modified column
    resizeSection(2, 100); // Size column
    resizeSection(3, 120); // Type column
}

void FileListHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
    // Use default painting for now - can be customized later if needed
    QHeaderView::paintSection(painter, rect, logicalIndex);
}

QSize FileListHeaderView::sectionSizeFromContents(int logicalIndex) const {
    // Return appropriate size based on column content with fixed height
    switch (logicalIndex) {
    case 0: // Name
        return QSize(250, 24);
    case 1: // Modified
        return QSize(150, 24);
    case 2: // Size
        return QSize(100, 24);
    case 3: // Type
        return QSize(120, 24);
    default:
        return QSize(100, 24);
    }
}

// FileListView implementation
FileListView::FileListView(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_headerView(nullptr)
    , m_listView(nullptr)
    , m_model(nullptr)
    , m_contextMenuController(nullptr)
    , m_selectionManager(nullptr)
    , m_currentSortOrder(Qt::AscendingOrder)
    , m_currentSortColumn(0)
    , m_viewMode(ViewMode::ListView)
    , m_listDelegate(nullptr)
    , m_iconDelegate(nullptr)
    , m_rubberBand(nullptr)
    , m_rubberBandActive(false)
{
    setupView();
}

FileListView::~FileListView() = default;

void FileListView::setupView() {
    // Create main layout
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    // Create header view
    m_headerView = new FileListHeaderView(Qt::Horizontal, this);
    m_layout->addWidget(m_headerView);
    
    // Create list view
    m_listView = new QListView(this);
    m_listView->setAlternatingRowColors(true);
    m_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listView->setUniformItemSizes(false);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create both delegates
    m_listDelegate = new FileItemDelegate(this);
    m_listDelegate->setHeaderView(m_headerView);
    m_iconDelegate = new FileIconDelegate(this);
    
    // Set initial delegate (list mode)
    m_listView->setItemDelegate(m_listDelegate);
    
    // Create selection manager
    m_selectionManager = new SelectionManager(this);
    
    // Create rubber band for drag selection
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    
    m_layout->addWidget(m_listView);
    
    setupConnections();
}

void FileListView::setupConnections() {
    // Connect list view signals
    connect(m_listView, &QListView::doubleClicked,
            this, &FileListView::onItemDoubleClicked);
    
    // Connect header signals
    connect(m_headerView, &QHeaderView::sectionClicked,
            this, &FileListView::onHeaderSectionClicked);
    
    // Connect header resize signals to update the view
    connect(m_headerView, &QHeaderView::sectionResized,
            this, [this]() {
                if (m_listView) {
                    m_listView->viewport()->update();
                }
            });
    
    // Connect selection manager signals
    if (m_selectionManager) {
        connect(m_selectionManager, &SelectionManager::selectionChanged,
                this, &FileListView::selectionChanged);
    }
    
    // Connect list view selection model
    if (m_listView && m_listView->selectionModel()) {
        connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &FileListView::onSelectionChanged);
    }
}

void FileListView::setModel(FileSystemModel* model) {
    m_model = model;
    if (m_listView) {
        m_listView->setModel(model);
    }
    
    if (m_headerView && model) {
        // Set the model for the header view
        m_headerView->setModel(model);
    }
    
    if (m_selectionManager && model) {
        // Initialize selection manager with model and selection model
        m_selectionManager->setModel(model);
        m_selectionManager->setSelectionModel(m_listView->selectionModel());
    }
}

void FileListView::setContextMenuController(ContextMenuController* controller) {
    m_contextMenuController = controller;
}

QModelIndex FileListView::currentIndex() const {
    return m_listView ? m_listView->currentIndex() : QModelIndex();
}

void FileListView::setCurrentIndex(const QModelIndex& index) {
    if (m_listView) {
        m_listView->setCurrentIndex(index);
    }
}

void FileListView::contextMenuEvent(QContextMenuEvent* event) {
    if (!m_contextMenuController) {
        return;
    }
    
    QModelIndex index;
    if (m_listView) {
        // Map the event position to the list view
        QPoint listViewPos = m_listView->mapFromParent(event->pos());
        if (m_listView->rect().contains(listViewPos)) {
            index = m_listView->indexAt(listViewPos);
        }
    }
    
    m_contextMenuController->showContextMenu(event->globalPos(), index);
}

void FileListView::onItemDoubleClicked(const QModelIndex& index) {
    if (!m_model || !index.isValid()) {
        return;
    }
    
    // Check if it's a group header
    if (m_model->isGroupHeader(index)) {
        // Toggle group expansion
        QString groupName = m_model->getGroupName(index);
        m_model->toggleGroupExpansion(groupName);
        return;
    }
    
    // Handle file/directory activation
    FileItem item = m_model->getFileItem(index);
    if (item.isDirectory) {
        emit directoryActivated(item.path);
    } else {
        // Open file with default application
        QDesktopServices::openUrl(QUrl::fromLocalFile(item.path));
    }
}

void FileListView::onHeaderSectionClicked(int logicalIndex) {
    if (!m_model) {
        return;
    }
    
    // Determine sort order based on current state
    Qt::SortOrder newOrder = Qt::AscendingOrder;
    if (m_currentSortColumn == logicalIndex) {
        // Same column clicked - toggle sort order
        newOrder = (m_currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        // Different column clicked - start with ascending
        newOrder = Qt::AscendingOrder;
    }
    
    m_currentSortColumn = logicalIndex;
    m_currentSortOrder = newOrder;
    
    // Create appropriate sorting strategy based on column
    std::unique_ptr<SortingStrategy> strategy;
    SortingStrategy::SortOrder strategyOrder = (newOrder == Qt::AscendingOrder) ? 
        SortingStrategy::SortOrder::Ascending : SortingStrategy::SortOrder::Descending;
    
    switch (logicalIndex) {
    case 0: // Name column
        strategy = std::make_unique<NameSortingStrategy>();
        break;
    case 1: // Modified column
        strategy = std::make_unique<ModifiedTimeSortingStrategy>();
        break;
    case 2: // Size column
        strategy = std::make_unique<SizeSortingStrategy>();
        break;
    case 3: // Type column
        strategy = std::make_unique<TypeSortingStrategy>();
        break;
    default:
        strategy = std::make_unique<NameSortingStrategy>();
        break;
    }
    
    // Apply sorting strategy to the model
    if (strategy) {
        m_model->setSortingStrategy(std::move(strategy), strategyOrder);
    }
    
    // Update header sort indicator
    m_headerView->setSortIndicator(logicalIndex, newOrder);
    
    // Emit signal to notify about sorting change (for toolbar sync)
    QString sortingType;
    switch (logicalIndex) {
    case 0: sortingType = "name"; break;
    case 1: sortingType = "modified"; break;
    case 2: sortingType = "size"; break;
    case 3: sortingType = "type"; break;
    }
    
    // Emit sorting change signal
    bool ascending = (newOrder == Qt::AscendingOrder);
    emit sortingChanged(sortingType, ascending);
}

// Multi-selection support methods
QModelIndexList FileListView::selectedIndexes() const {
    return m_selectionManager ? m_selectionManager->selectedFileIndexes() : QModelIndexList();
}

void FileListView::selectGroup(const QString& groupName) {
    if (m_selectionManager) {
        m_selectionManager->selectGroup(groupName);
    }
}

void FileListView::clearSelection() {
    if (m_selectionManager) {
        m_selectionManager->clearSelection();
    }
}

void FileListView::setSelectionMode(QAbstractItemView::SelectionMode mode) {
    if (m_listView) {
        m_listView->setSelectionMode(mode);
    }
    
    if (m_selectionManager) {
        SelectionManager::SelectionMode managerMode;
        switch (mode) {
        case QAbstractItemView::SingleSelection:
            managerMode = SelectionManager::Single;
            break;
        case QAbstractItemView::ExtendedSelection:
            managerMode = SelectionManager::Extended;
            break;
        case QAbstractItemView::MultiSelection:
            managerMode = SelectionManager::Multi;
            break;
        default:
            managerMode = SelectionManager::Extended;
            break;
        }
        m_selectionManager->setSelectionMode(managerMode);
    }
}

void FileListView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Check if we clicked on list view area
        QPoint listViewPos = m_listView->mapFromParent(event->pos());
        if (m_listView->rect().contains(listViewPos)) {
            QModelIndex index = m_listView->indexAt(listViewPos);
            
            if (index.isValid()) {
                // Check if it's a group header
                if (isGroupHeader(index)) {
                    handleGroupHeaderClick(index);
                    return;
                }
                
                // Handle file item click with modifiers
                if (m_selectionManager) {
                    m_selectionManager->handleClick(index, event->modifiers());
                }
            } else {
                // Clicked on empty area - start rubber band selection
                if (!(event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
                    clearSelection();
                }
                
                m_rubberBandOrigin = event->pos();
                m_rubberBandActive = true;
                if (m_rubberBand) {
                    m_rubberBand->setGeometry(QRect(m_rubberBandOrigin, QSize()));
                    m_rubberBand->show();
                }
            }
        }
    }
    
    QWidget::mousePressEvent(event);
}

void FileListView::mouseMoveEvent(QMouseEvent* event) {
    if (m_rubberBandActive && m_rubberBand) {
        QRect rect = QRect(m_rubberBandOrigin, event->pos()).normalized();
        m_rubberBand->setGeometry(rect);
        updateRubberBandSelection();
    }
    
    QWidget::mouseMoveEvent(event);
}

void FileListView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_rubberBandActive) {
        m_rubberBandActive = false;
        if (m_rubberBand) {
            m_rubberBand->hide();
        }
    }
    
    QWidget::mouseReleaseEvent(event);
}

void FileListView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_A && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+A: Select all files
        if (m_selectionManager && m_model) {
            QModelIndexList allFileIndexes;
            for (int i = 0; i < m_model->rowCount(); ++i) {
                QModelIndex index = m_model->index(i, 0);
                if (index.isValid() && !m_model->isGroupHeader(index)) {
                    allFileIndexes.append(index);
                }
            }
            m_selectionManager->selectIndexes(allFileIndexes, true);
        }
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);
}

void FileListView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    
    if (m_selectionManager) {
        emit selectionChanged(m_selectionManager->selectedFileIndexes());
    }
}

void FileListView::onGroupHeaderClicked(const QModelIndex& index) {
    handleGroupHeaderClick(index);
}

void FileListView::updateRubberBandSelection() {
    if (!m_rubberBand || !m_selectionManager) {
        return;
    }
    
    QRect rubberBandRect = m_rubberBand->geometry();
    QRect listViewRect = m_listView->geometry();
    
    // Convert rubber band rect to list view coordinates
    QRect selectionRect = QRect(
        rubberBandRect.topLeft() - listViewRect.topLeft(),
        rubberBandRect.size()
    );
    
    m_selectionManager->selectInRect(selectionRect, m_listView);
}

QModelIndexList FileListView::getIndexesInRect(const QRect& rect) const {
    if (!m_model) {
        return QModelIndexList();
    }
    
    QModelIndexList indexes;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        if (index.isValid()) {
            QRect itemRect = m_listView->visualRect(index);
            if (rect.intersects(itemRect) && !m_model->isGroupHeader(index)) {
                indexes.append(index);
            }
        }
    }
    
    return indexes;
}

bool FileListView::isGroupHeader(const QModelIndex& index) const {
    return m_model && m_model->isGroupHeader(index);
}

void FileListView::handleGroupHeaderClick(const QModelIndex& index) {
    if (!m_model || !m_selectionManager) {
        return;
    }
    
    QString groupName = m_model->getGroupName(index);
    if (!groupName.isEmpty()) {
        // Select all files in this group
        m_selectionManager->selectGroup(groupName);
    }
}

void FileListView::setViewMode(ViewMode mode) {
    if (m_viewMode == mode) {
        return;
    }
    
    m_viewMode = mode;
    
    if (mode == ViewMode::ListView) {
        switchToListMode();
    } else {
        switchToIconMode();
    }
    
    emit viewModeChanged(mode);
}

void FileListView::switchToListMode() {
    if (!m_listView || !m_listDelegate) {
        return;
    }
    
    // Show header view
    if (m_headerView) {
        m_headerView->show();
    }
    
    // Configure list view for list mode
    m_listView->setViewMode(QListView::ListMode);
    m_listView->setFlow(QListView::TopToBottom);
    m_listView->setWrapping(false);
    m_listView->setResizeMode(QListView::Fixed);
    m_listView->setMovement(QListView::Static);
    m_listView->setUniformItemSizes(false);
    m_listView->setSpacing(0);
    
    // Set list delegate
    m_listView->setItemDelegate(m_listDelegate);
}

void FileListView::switchToIconMode() {
    if (!m_listView || !m_iconDelegate) {
        return;
    }
    
    // Hide header view in icon mode
    if (m_headerView) {
        m_headerView->hide();
    }
    
    // Configure list view for icon mode
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setFlow(QListView::LeftToRight);
    m_listView->setWrapping(true);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setMovement(QListView::Static);
    m_listView->setUniformItemSizes(false);
    m_listView->setSpacing(5);
    
    // Set icon delegate
    m_listView->setItemDelegate(m_iconDelegate);
} 