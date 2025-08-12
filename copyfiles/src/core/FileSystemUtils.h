#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <QString>
#include <QStringList>
#include <QtGlobal>

/**
 * @brief Utility functions for file system operations
 * 
 * This class provides helper functions for file and directory operations
 * that are commonly needed by copy algorithms.
 */
class FileSystemUtils
{
public:
    /**
     * @brief Calculate total size of a file or directory
     * @param path Path to file or directory
     * @return Total size in bytes, -1 on error
     */
    static qint64 calculateTotalSize(const QString& path);
    
    /**
     * @brief Get list of all files in a directory recursively
     * @param dirPath Directory path
     * @param files Output list of file paths
     * @return true on success, false on error
     */
    static bool getFileList(const QString& dirPath, QStringList& files);
    
    /**
     * @brief Validate and normalize a file path
     * @param path Input path
     * @return Normalized path, empty string on error
     */
    static QString normalizePath(const QString& path);
    
    /**
     * @brief Check if a path exists and is accessible
     * @param path Path to check
     * @return true if path exists and is accessible
     */
    static bool isPathAccessible(const QString& path);
    
    /**
     * @brief Create directory structure if it doesn't exist
     * @param path Directory path to create
     * @return true on success, false on error
     */
    static bool ensureDirectoryExists(const QString& path);
    
    /**
     * @brief Get human-readable error message for errno
     * @param errorCode errno value
     * @return Error description string
     */
    static QString getErrorString(int errorCode);
    
    /**
     * @brief Check if enough space is available for copy operation
     * @param destinationPath Destination directory
     * @param requiredBytes Number of bytes needed
     * @return true if enough space available
     */
    static bool hasEnoughSpace(const QString& destinationPath, qint64 requiredBytes);

private:
    // Private constructor - this is a utility class
    FileSystemUtils() = default;
    
    static qint64 calculateDirectorySize(const QString& dirPath);
};

#endif // FILESYSTEMUTILS_H 