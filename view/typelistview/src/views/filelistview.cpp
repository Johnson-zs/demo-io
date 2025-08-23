#include "filelistview.h"
#include "fileitemdelegate.h"
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
    , m_currentSortOrder(Qt::AscendingOrder)
    , m_currentSortColumn(0)
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
    m_listView->setUniformItemSizes(false);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Set custom delegate and connect it to header view
    FileItemDelegate* delegate = new FileItemDelegate(this);
    delegate->setHeaderView(m_headerView);
    m_listView->setItemDelegate(delegate);
    
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