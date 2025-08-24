#include "groupingstrategy.h"
#include <QObject>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>
#include <QLocale>
#include <algorithm>

// Base GroupingStrategy implementation
int GroupingStrategy::getGroupDisplayOrder(const QString& groupName, GroupOrder order) const {
    QStringList orderList = getGroupOrder(order);
    int index = orderList.indexOf(groupName);
    return index >= 0 ? index : 999; // Unknown groups go to the end
}

// NoGroupingStrategy implementation
QString NoGroupingStrategy::getGroupName(const FileItem& item) const {
    Q_UNUSED(item)
    return QString(); // No grouping
}

QStringList NoGroupingStrategy::getGroupOrder(GroupOrder order) const {
    Q_UNUSED(order)
    return QStringList(); // No groups
}

bool NoGroupingStrategy::isGroupVisible(const QString& groupName, const QList<FileItem>& items) const {
    Q_UNUSED(groupName)
    return !items.isEmpty();
}

QString NoGroupingStrategy::getStrategyName() const {
    return QObject::tr("None");
}

// TypeGroupingStrategy implementation
QString TypeGroupingStrategy::getGroupName(const FileItem& item) const {
    return getTypeFromMimeType(item.type, item.isDirectory);
}

QStringList TypeGroupingStrategy::getGroupOrder(GroupOrder order) const {
    QStringList baseOrder{
        QObject::tr("Directory"),
        QObject::tr("Document"),
        QObject::tr("Image"),
        QObject::tr("Video"),
        QObject::tr("Audio"),
        QObject::tr("Archive"),
        QObject::tr("Application"),
        QObject::tr("Executable"),
        QObject::tr("Unknown")
    };
    
    if (order == GroupOrder::Descending) {
        std::reverse(baseOrder.begin(), baseOrder.end());
    }
    
    return baseOrder;
}

bool TypeGroupingStrategy::isGroupVisible(const QString& groupName, const QList<FileItem>& items) const {
    Q_UNUSED(groupName)
    return !items.isEmpty();
}

QString TypeGroupingStrategy::getStrategyName() const {
    return QObject::tr("Type");
}

int TypeGroupingStrategy::getGroupDisplayOrder(const QString& groupName, GroupOrder order) const {
    QStringList orderList = getGroupOrder(order);
    int index = orderList.indexOf(groupName);
    return index >= 0 ? index : 999;
}

QString TypeGroupingStrategy::getTypeFromMimeType(const QString& mimeType, bool isDirectory) const {
    if (isDirectory) {
        return QObject::tr("Directory");
    }
    
    if (mimeType.startsWith("text/") || 
        mimeType == "application/pdf" ||
        mimeType.startsWith("application/vnd.ms-") ||
        mimeType.startsWith("application/vnd.openxmlformats-") ||
        mimeType.startsWith("application/vnd.oasis.opendocument")) {
        return QObject::tr("Document");
    }
    
    if (mimeType.startsWith("image/")) {
        return QObject::tr("Image");
    }
    
    if (mimeType.startsWith("video/")) {
        return QObject::tr("Video");
    }
    
    if (mimeType.startsWith("audio/")) {
        return QObject::tr("Audio");
    }
    
    if (mimeType.startsWith("application/zip") ||
        mimeType.startsWith("application/x-tar") ||
        mimeType.startsWith("application/x-gzip") ||
        mimeType.startsWith("application/x-bzip") ||
        mimeType.startsWith("application/x-7z") ||
        mimeType.startsWith("application/x-rar")) {
        return QObject::tr("Archive");
    }
    
    if (mimeType == "application/x-executable" ||
        mimeType == "application/x-sharedlib") {
        return QObject::tr("Executable");
    }
    
    if (mimeType.startsWith("application/")) {
        return QObject::tr("Application");
    }
    
    return QObject::tr("Unknown");
}

// TimeGroupingStrategy implementation
TimeGroupingStrategy::TimeGroupingStrategy(TimeType timeType) : m_timeType(timeType) {
}

QString TimeGroupingStrategy::getGroupName(const FileItem& item) const {
    QDateTime dateTime = (m_timeType == ModificationTime) ? item.dateModified : item.dateCreated;
    return getTimeGroupName(dateTime);
}

QStringList TimeGroupingStrategy::getGroupOrder(GroupOrder order) const {
    QStringList baseOrder{
        QObject::tr("Today"),
        QObject::tr("Yesterday"),
        QObject::tr("Past 7 days"),
        QObject::tr("Past 30 days"),
        QObject::tr("January"), QObject::tr("February"), QObject::tr("March"),
        QObject::tr("April"), QObject::tr("May"), QObject::tr("June"),
        QObject::tr("July"), QObject::tr("August"), QObject::tr("September"),
        QObject::tr("October"), QObject::tr("November"), QObject::tr("December"),
        QObject::tr("2024"), QObject::tr("2023"), QObject::tr("2022"), 
        QObject::tr("2021"), QObject::tr("2020"),
        QObject::tr("Earlier")
    };
    
    if (order == GroupOrder::Descending) {
        std::reverse(baseOrder.begin(), baseOrder.end());
    }
    
    return baseOrder;
}

bool TimeGroupingStrategy::isGroupVisible(const QString& groupName, const QList<FileItem>& items) const {
    Q_UNUSED(groupName)
    return !items.isEmpty();
}

QString TimeGroupingStrategy::getStrategyName() const {
    return (m_timeType == ModificationTime) ? 
        QObject::tr("Modification Time") : QObject::tr("Creation Time");
}

int TimeGroupingStrategy::getGroupDisplayOrder(const QString& groupName, GroupOrder order) const {
    QStringList orderList = getGroupOrder(order);
    int index = orderList.indexOf(groupName);
    return index >= 0 ? index : 999;
}

QString TimeGroupingStrategy::getTimeGroupName(const QDateTime& dateTime) const {
    if (!dateTime.isValid()) {
        return QObject::tr("Unknown");
    }
    
    QDateTime now = QDateTime::currentDateTime();
    QDate fileDate = dateTime.date();
    QDate today = now.date();
    
    // Today
    if (fileDate == today) {
        return QObject::tr("Today");
    }
    
    // Yesterday
    if (fileDate == today.addDays(-1)) {
        return QObject::tr("Yesterday");
    }
    
    // Past 7 days
    if (fileDate >= today.addDays(-7)) {
        return QObject::tr("Past 7 days");
    }
    
    // Past 30 days
    if (fileDate >= today.addDays(-30)) {
        return QObject::tr("Past 30 days");
    }
    
    // This year - by month
    if (fileDate.year() == today.year()) {
        QStringList months = {
            QObject::tr("January"), QObject::tr("February"), QObject::tr("March"),
            QObject::tr("April"), QObject::tr("May"), QObject::tr("June"),
            QObject::tr("July"), QObject::tr("August"), QObject::tr("September"),
            QObject::tr("October"), QObject::tr("November"), QObject::tr("December")
        };
        return months[fileDate.month() - 1];
    }
    
    // Past 5 years
    int currentYear = today.year();
    int fileYear = fileDate.year();
    if (fileYear >= currentYear - 5 && fileYear < currentYear) {
        return QString::number(fileYear);
    }
    
    // Earlier
    return QObject::tr("Earlier");
}

// NameGroupingStrategy implementation
QString NameGroupingStrategy::getGroupName(const FileItem& item) const {
    return getNameGroup(item.name);
}

QStringList NameGroupingStrategy::getGroupOrder(GroupOrder order) const {
    QStringList baseOrder{
        QObject::tr("0-9"),
        QObject::tr("A-H"),
        QObject::tr("I-P"),
        QObject::tr("Q-Z"),
        QObject::tr("Pinyin A-H"),
        QObject::tr("Pinyin I-P"),
        QObject::tr("Pinyin Q-Z"),
        QObject::tr("Other")
    };
    
    if (order == GroupOrder::Descending) {
        std::reverse(baseOrder.begin(), baseOrder.end());
    }
    
    return baseOrder;
}

bool NameGroupingStrategy::isGroupVisible(const QString& groupName, const QList<FileItem>& items) const {
    Q_UNUSED(groupName)
    return !items.isEmpty();
}

QString NameGroupingStrategy::getStrategyName() const {
    return QObject::tr("Name");
}

int NameGroupingStrategy::getGroupDisplayOrder(const QString& groupName, GroupOrder order) const {
    QStringList orderList = getGroupOrder(order);
    int index = orderList.indexOf(groupName);
    return index >= 0 ? index : 999;
}

QString NameGroupingStrategy::getNameGroup(const QString& name) const {
    if (name.isEmpty()) {
        return QObject::tr("Other");
    }
    
    QChar firstChar = name[0].toUpper();
    
    // Numbers
    if (firstChar.isDigit()) {
        return QObject::tr("0-9");
    }
    
    // English letters
    if (firstChar >= 'A' && firstChar <= 'Z') {
        if (firstChar <= 'H') {
            return QObject::tr("A-H");
        } else if (firstChar <= 'P') {
            return QObject::tr("I-P");
        } else {
            return QObject::tr("Q-Z");
        }
    }
    
    // Chinese characters (simplified approach)
    if (isChineseCharacter(firstChar)) {
        QString pinyinGroup = getPinyinGroup(firstChar);
        if (!pinyinGroup.isEmpty()) {
            return pinyinGroup;
        }
    }
    
    return QObject::tr("Other");
}

bool NameGroupingStrategy::isChineseCharacter(const QChar& ch) const {
    // Basic check for Chinese characters (CJK Unified Ideographs)
    ushort unicode = ch.unicode();
    return (unicode >= 0x4E00 && unicode <= 0x9FFF);
}

QString NameGroupingStrategy::getPinyinGroup(const QChar& ch) const {
    // Simplified pinyin grouping based on Unicode ranges
    // This is a basic implementation - a full implementation would use a pinyin library
    ushort unicode = ch.unicode();
    
    // Very basic grouping based on common character ranges
    // This is not accurate for all characters but provides a basic grouping
    if (unicode >= 0x4E00 && unicode <= 0x5FFF) {
        return QObject::tr("Pinyin A-H");
    } else if (unicode >= 0x6000 && unicode <= 0x7FFF) {
        return QObject::tr("Pinyin I-P");
    } else if (unicode >= 0x8000 && unicode <= 0x9FFF) {
        return QObject::tr("Pinyin Q-Z");
    }
    
    return QObject::tr("Pinyin A-H"); // Default fallback
}

// SizeGroupingStrategy implementation
QString SizeGroupingStrategy::getGroupName(const FileItem& item) const {
    return getSizeGroup(item.size, item.isDirectory);
}

QStringList SizeGroupingStrategy::getGroupOrder(GroupOrder order) const {
    QStringList baseOrder{
        QObject::tr("Unknown"),
        QObject::tr("Empty"),
        QObject::tr("Tiny"),
        QObject::tr("Small"),
        QObject::tr("Medium"),
        QObject::tr("Large"),
        QObject::tr("Huge"),
        QObject::tr("Massive")
    };
    
    if (order == GroupOrder::Descending) {
        std::reverse(baseOrder.begin(), baseOrder.end());
    }
    
    return baseOrder;
}

bool SizeGroupingStrategy::isGroupVisible(const QString& groupName, const QList<FileItem>& items) const {
    Q_UNUSED(groupName)
    return !items.isEmpty();
}

QString SizeGroupingStrategy::getStrategyName() const {
    return QObject::tr("Size");
}

int SizeGroupingStrategy::getGroupDisplayOrder(const QString& groupName, GroupOrder order) const {
    QStringList orderList = getGroupOrder(order);
    int index = orderList.indexOf(groupName);
    return index >= 0 ? index : 999;
}

QString SizeGroupingStrategy::getSizeGroup(qint64 size, bool isDirectory) const {
    if (isDirectory) {
        return QObject::tr("Unknown");
    }
    
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size == 0) {
        return QObject::tr("Empty");
    } else if (size < 16 * KB) {
        return QObject::tr("Tiny");
    } else if (size < MB) {
        return QObject::tr("Small");
    } else if (size < 128 * MB) {
        return QObject::tr("Medium");
    } else if (size < GB) {
        return QObject::tr("Large");
    } else if (size < 4 * GB) {
        return QObject::tr("Huge");
    } else {
        return QObject::tr("Massive");
    }
} 