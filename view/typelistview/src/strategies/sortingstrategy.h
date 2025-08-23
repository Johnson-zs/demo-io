#ifndef SORTINGSTRATEGY_H
#define SORTINGSTRATEGY_H

#include <functional>
#include <QString>
#include <QList>

// Forward declaration
struct FileItem;

/**
 * @brief Abstract base class for file sorting strategies
 * 
 * This class defines the interface for different sorting algorithms
 * that can be applied to file lists.
 */
class SortingStrategy
{
public:
    enum class SortOrder { Ascending, Descending };

    virtual ~SortingStrategy() = default;
    
    /**
     * @brief Get a comparator function for sorting
     * @param order Sort order (ascending or descending)
     * @return Comparator function
     */
    virtual std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const = 0;
    
    /**
     * @brief Get the name of this sorting strategy
     * @return Strategy name
     */
    virtual QString getStrategyName() const = 0;
    
    /**
     * @brief Check if this strategy can sort the given items
     * @param items List of items to check
     * @return True if sorting is possible
     */
    virtual bool canSort(const QList<FileItem>& items) const;
};

/**
 * @brief Sort files by name (alphabetically)
 */
class NameSortingStrategy : public SortingStrategy
{
public:
    std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const override;
    QString getStrategyName() const override;
};

/**
 * @brief Sort files by modification time
 */
class ModifiedTimeSortingStrategy : public SortingStrategy
{
public:
    std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const override;
    QString getStrategyName() const override;
};

/**
 * @brief Sort files by creation time
 */
class CreatedTimeSortingStrategy : public SortingStrategy
{
public:
    std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const override;
    QString getStrategyName() const override;
};

/**
 * @brief Sort files by size
 */
class SizeSortingStrategy : public SortingStrategy
{
public:
    std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const override;
    QString getStrategyName() const override;
};

/**
 * @brief Sort files by type
 */
class TypeSortingStrategy : public SortingStrategy
{
public:
    std::function<bool(const FileItem&, const FileItem&)> getComparator(SortOrder order = SortOrder::Ascending) const override;
    QString getStrategyName() const override;
};

#endif // SORTINGSTRATEGY_H 