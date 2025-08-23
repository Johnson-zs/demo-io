#ifndef GROUPINGSTRATEGY_H
#define GROUPINGSTRATEGY_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>
#include <memory>
#include "../core/fileitem.h"

/**
 * @brief Abstract base class for file grouping strategies
 * 
 * This interface defines the contract for different grouping mechanisms.
 * Concrete implementations determine how files are categorized into groups.
 */
class GroupingStrategy {
public:
    virtual ~GroupingStrategy() = default;
    
    /**
     * @brief Get the group name for a given file item
     * @param item The file item to categorize
     * @return The group name this item belongs to
     */
    virtual QString getGroupName(const FileItem& item) const = 0;
    
    /**
     * @brief Get the ordered list of all possible group names
     * @return List of group names in display order
     */
    virtual QStringList getGroupOrder() const = 0;
    
    /**
     * @brief Check if a group should be visible
     * @param groupName The name of the group
     * @param items The items in this group
     * @return True if the group should be displayed
     */
    virtual bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const = 0;
    
    /**
     * @brief Get the strategy name for UI display
     * @return Human-readable strategy name
     */
    virtual QString getStrategyName() const = 0;
    
    /**
     * @brief Get the display order for a group
     * @param groupName The group name
     * @return Display order (lower numbers appear first)
     */
    virtual int getGroupDisplayOrder(const QString& groupName) const;
};

/**
 * @brief No grouping strategy - all files in one virtual group
 */
class NoGroupingStrategy : public GroupingStrategy {
public:
    QString getGroupName(const FileItem& item) const override;
    QStringList getGroupOrder() const override;
    bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const override;
    QString getStrategyName() const override;
};

/**
 * @brief Group files by MIME type/file type
 */
class TypeGroupingStrategy : public GroupingStrategy {
public:
    QString getGroupName(const FileItem& item) const override;
    QStringList getGroupOrder() const override;
    bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const override;
    QString getStrategyName() const override;
    int getGroupDisplayOrder(const QString& groupName) const override;

private:
    QString getTypeFromMimeType(const QString& mimeType, bool isDirectory) const;
};

/**
 * @brief Group files by modification or creation time
 */
class TimeGroupingStrategy : public GroupingStrategy {
public:
    enum TimeType { ModificationTime, CreationTime };
    
    explicit TimeGroupingStrategy(TimeType timeType = ModificationTime);
    
    QString getGroupName(const FileItem& item) const override;
    QStringList getGroupOrder() const override;
    bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const override;
    QString getStrategyName() const override;
    int getGroupDisplayOrder(const QString& groupName) const override;

private:
    TimeType m_timeType;
    QString getTimeGroupName(const QDateTime& dateTime) const;
};

/**
 * @brief Group files by first letter of name
 */
class NameGroupingStrategy : public GroupingStrategy {
public:
    QString getGroupName(const FileItem& item) const override;
    QStringList getGroupOrder() const override;
    bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const override;
    QString getStrategyName() const override;
    int getGroupDisplayOrder(const QString& groupName) const override;

private:
    QString getNameGroup(const QString& name) const;
    bool isChineseCharacter(const QChar& ch) const;
    QString getPinyinGroup(const QChar& ch) const;
};

/**
 * @brief Group files by file size
 */
class SizeGroupingStrategy : public GroupingStrategy {
public:
    QString getGroupName(const FileItem& item) const override;
    QStringList getGroupOrder() const override;
    bool isGroupVisible(const QString& groupName, const QList<FileItem>& items) const override;
    QString getStrategyName() const override;
    int getGroupDisplayOrder(const QString& groupName) const override;

private:
    QString getSizeGroup(qint64 size, bool isDirectory) const;
};

#endif // GROUPINGSTRATEGY_H 