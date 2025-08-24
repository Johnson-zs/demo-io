#include "selectionmanager.h"
#include "../models/filesystemmodel.h"
#include <QAbstractItemView>
#include <QDebug>

SelectionManager::SelectionManager(QObject* parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_selectionModel(nullptr)
    , m_selectionMode(Extended)
{
}

SelectionManager::~SelectionManager() = default;

void SelectionManager::setModel(FileSystemModel* model) {
    m_model = model;
}

void SelectionManager::setSelectionModel(QItemSelectionModel* selectionModel) {
    if (m_selectionModel) {
        disconnect(m_selectionModel, &QItemSelectionModel::selectionChanged,
                   this, &SelectionManager::onSelectionModelChanged);
    }
    
    m_selectionModel = selectionModel;
    
    if (m_selectionModel) {
        connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
                this, &SelectionManager::onSelectionModelChanged);
    }
}

void SelectionManager::selectIndex(const QModelIndex& index, bool clearPrevious) {
    if (!m_selectionModel || !index.isValid() || !shouldSelectIndex(index)) {
        return;
    }
    
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select;
    if (clearPrevious) {
        flags |= QItemSelectionModel::Clear;
    }
    
    m_selectionModel->select(index, flags);
    updateLastSelectedIndex(index);
}

void SelectionManager::selectIndexes(const QModelIndexList& indexes, bool clearPrevious) {
    if (!m_selectionModel || indexes.isEmpty()) {
        return;
    }
    
    QItemSelection selection;
    for (const QModelIndex& index : indexes) {
        if (index.isValid() && shouldSelectIndex(index)) {
            selection.select(index, index);
        }
    }
    
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select;
    if (clearPrevious) {
        flags |= QItemSelectionModel::Clear;
    }
    
    m_selectionModel->select(selection, flags);
    
    if (!indexes.isEmpty()) {
        updateLastSelectedIndex(indexes.last());
    }
}

void SelectionManager::selectGroup(const QString& groupName) {
    if (!m_model || groupName.isEmpty()) {
        return;
    }
    
    QModelIndexList groupIndexes = getGroupFileIndexes(groupName);
    selectIndexes(groupIndexes, false); // Don't clear previous selection for group selection
}

void SelectionManager::selectRange(const QModelIndex& start, const QModelIndex& end) {
    if (!m_selectionModel || !start.isValid() || !end.isValid()) {
        return;
    }
    
    QModelIndexList rangeIndexes = getIndexesInRange(start, end);
    selectIndexes(rangeIndexes, false);
}

void SelectionManager::selectInRect(const QRect& rect, const QAbstractItemView* view) {
    if (!m_model || !view) {
        return;
    }
    
    QModelIndexList indexesInRect;
    
    // Iterate through all visible items and check if they intersect with the rect
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        if (!index.isValid()) {
            continue;
        }
        
        QRect itemRect = view->visualRect(index);
        if (rect.intersects(itemRect) && shouldSelectIndex(index)) {
            indexesInRect.append(index);
        }
    }
    
    selectIndexes(indexesInRect, false);
}

void SelectionManager::toggleSelection(const QModelIndex& index) {
    if (!m_selectionModel || !index.isValid() || !shouldSelectIndex(index)) {
        return;
    }
    
    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Toggle;
    m_selectionModel->select(index, flags);
    updateLastSelectedIndex(index);
}

void SelectionManager::clearSelection() {
    if (m_selectionModel) {
        m_selectionModel->clearSelection();
    }
    m_lastSelectedIndex = QModelIndex();
    m_currentSelection.clear();
}

QModelIndexList SelectionManager::selectedIndexes() const {
    if (m_selectionModel) {
        return m_selectionModel->selectedIndexes();
    }
    return QModelIndexList();
}

QModelIndexList SelectionManager::selectedFileIndexes() const {
    QModelIndexList fileIndexes;
    QModelIndexList allSelected = selectedIndexes();
    
    for (const QModelIndex& index : allSelected) {
        if (shouldSelectIndex(index)) {
            fileIndexes.append(index);
        }
    }
    
    return fileIndexes;
}

bool SelectionManager::isSelected(const QModelIndex& index) const {
    return m_selectionModel && m_selectionModel->isSelected(index);
}

int SelectionManager::selectedCount() const {
    return selectedFileIndexes().count();
}

void SelectionManager::handleClick(const QModelIndex& index, Qt::KeyboardModifiers modifiers) {
    if (!index.isValid()) {
        return;
    }
    
    if (modifiers & Qt::ControlModifier) {
        // Ctrl+Click: Toggle selection
        toggleSelection(index);
    } else if (modifiers & Qt::ShiftModifier && m_lastSelectedIndex.isValid()) {
        // Shift+Click: Range selection
        selectRange(m_lastSelectedIndex, index);
    } else {
        // Normal click: Select single item
        selectIndex(index, true);
    }
}

void SelectionManager::handleRangeSelection(const QModelIndex& start, const QModelIndex& end) {
    selectRange(start, end);
}

void SelectionManager::setSelectionMode(SelectionMode mode) {
    m_selectionMode = mode;
}

void SelectionManager::onSelectionModelChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    
    m_currentSelection = selectedFileIndexes();
    emit selectionChanged(m_currentSelection);
}

QModelIndexList SelectionManager::getGroupFileIndexes(const QString& groupName) const {
    if (!m_model) {
        return QModelIndexList();
    }
    
    QModelIndexList groupIndexes;
    
    // Iterate through all model items to find files in the specified group
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        if (!index.isValid()) {
            continue;
        }
        
        // Skip group headers
        if (m_model->isGroupHeader(index)) {
            continue;
        }
        
        // Check if this file belongs to the target group
        QString itemGroupName = m_model->getGroupName(index);
        if (itemGroupName == groupName) {
            groupIndexes.append(index);
        }
    }
    
    return groupIndexes;
}

QModelIndexList SelectionManager::getIndexesInRange(const QModelIndex& start, const QModelIndex& end) const {
    if (!m_model || !start.isValid() || !end.isValid()) {
        return QModelIndexList();
    }
    
    QModelIndexList rangeIndexes;
    
    int startRow = start.row();
    int endRow = end.row();
    
    // Ensure proper order
    if (startRow > endRow) {
        qSwap(startRow, endRow);
    }
    
    // Collect all valid indexes in the range
    for (int row = startRow; row <= endRow; ++row) {
        QModelIndex index = m_model->index(row, 0);
        if (index.isValid() && shouldSelectIndex(index)) {
            rangeIndexes.append(index);
        }
    }
    
    return rangeIndexes;
}

bool SelectionManager::shouldSelectIndex(const QModelIndex& index) const {
    if (!m_model || !index.isValid()) {
        return false;
    }
    
    // Don't select group headers - only select actual file items
    return !m_model->isGroupHeader(index);
}

void SelectionManager::updateLastSelectedIndex(const QModelIndex& index) {
    if (shouldSelectIndex(index)) {
        m_lastSelectedIndex = index;
    }
} 