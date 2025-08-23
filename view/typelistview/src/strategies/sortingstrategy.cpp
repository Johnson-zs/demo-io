#include "sortingstrategy.h"
#include "../core/fileitem.h"
#include <QObject>
#include <QString>
#include <QDateTime>

// Base SortingStrategy implementation
bool SortingStrategy::canSort(const QList<FileItem>& items) const {
    Q_UNUSED(items)
    return true; // Most strategies can sort any items
}

// NameSortingStrategy implementation
std::function<bool(const FileItem&, const FileItem&)> NameSortingStrategy::getComparator(SortOrder order) const {
    return [order](const FileItem& a, const FileItem& b) -> bool {
        // Directories always come first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory;
        }
        
        // Compare names case-insensitively
        int result = QString::compare(a.name, b.name, Qt::CaseInsensitive);
        
        if (order == SortOrder::Ascending) {
            return result < 0;
        } else {
            return result > 0;
        }
    };
}

QString NameSortingStrategy::getStrategyName() const {
    return QObject::tr("Name");
}

// ModifiedTimeSortingStrategy implementation
std::function<bool(const FileItem&, const FileItem&)> ModifiedTimeSortingStrategy::getComparator(SortOrder order) const {
    return [order](const FileItem& a, const FileItem& b) -> bool {
        // Directories always come first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory;
        }
        
        // Compare modification times
        if (order == SortOrder::Ascending) {
            return a.dateModified < b.dateModified;
        } else {
            return a.dateModified > b.dateModified;
        }
    };
}

QString ModifiedTimeSortingStrategy::getStrategyName() const {
    return QObject::tr("Date Modified");
}

// CreatedTimeSortingStrategy implementation
std::function<bool(const FileItem&, const FileItem&)> CreatedTimeSortingStrategy::getComparator(SortOrder order) const {
    return [order](const FileItem& a, const FileItem& b) -> bool {
        // Directories always come first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory;
        }
        
        // Compare creation times
        if (order == SortOrder::Ascending) {
            return a.dateCreated < b.dateCreated;
        } else {
            return a.dateCreated > b.dateCreated;
        }
    };
}

QString CreatedTimeSortingStrategy::getStrategyName() const {
    return QObject::tr("Date Created");
}

// SizeSortingStrategy implementation
std::function<bool(const FileItem&, const FileItem&)> SizeSortingStrategy::getComparator(SortOrder order) const {
    return [order](const FileItem& a, const FileItem& b) -> bool {
        // Directories always come first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory;
        }
        
        // For directories, sort by name since size is not meaningful
        if (a.isDirectory && b.isDirectory) {
            int result = QString::compare(a.name, b.name, Qt::CaseInsensitive);
            return order == SortOrder::Ascending ? result < 0 : result > 0;
        }
        
        // Compare file sizes
        if (order == SortOrder::Ascending) {
            return a.size < b.size;
        } else {
            return a.size > b.size;
        }
    };
}

QString SizeSortingStrategy::getStrategyName() const {
    return QObject::tr("Size");
}

// TypeSortingStrategy implementation
std::function<bool(const FileItem&, const FileItem&)> TypeSortingStrategy::getComparator(SortOrder order) const {
    return [order](const FileItem& a, const FileItem& b) -> bool {
        // Directories always come first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory;
        }
        
        // Get type strings for comparison
        QString typeA = a.getTypeString();
        QString typeB = b.getTypeString();
        
        // Compare types
        int result = QString::compare(typeA, typeB, Qt::CaseInsensitive);
        
        // If types are the same, sort by name
        if (result == 0) {
            result = QString::compare(a.name, b.name, Qt::CaseInsensitive);
        }
        
        if (order == SortOrder::Ascending) {
            return result < 0;
        } else {
            return result > 0;
        }
    };
}

QString TypeSortingStrategy::getStrategyName() const {
    return QObject::tr("Type");
} 