#ifndef COPYALGORITHM_H
#define COPYALGORITHM_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include "ProgressObserver.h"

/**
 * @brief Abstract base class for file copy algorithms
 * 
 * This class provides the interface that all copy algorithms must implement.
 * It supports copying both individual files and entire directories, with
 * progress reporting and pause/resume functionality.
 */
class CopyAlgorithm : public QObject
{
    Q_OBJECT
    
public:
    explicit CopyAlgorithm(QObject *parent = nullptr);
    virtual ~CopyAlgorithm() = default;
    
    /**
     * @brief Copy a single file from source to destination
     * @param source Path to source file
     * @param dest Path to destination file
     * @param observer Optional progress observer
     * @return true if successful, false otherwise
     */
    virtual bool copyFile(const QString& source, const QString& dest, 
                         ProgressObserver* observer = nullptr) = 0;
    
    /**
     * @brief Copy an entire directory from source to destination
     * @param source Path to source directory
     * @param dest Path to destination directory
     * @param observer Optional progress observer
     * @return true if successful, false otherwise
     */
    virtual bool copyDirectory(const QString& source, const QString& dest,
                              ProgressObserver* observer = nullptr) = 0;
    
    /**
     * @brief Calculate total size of files to be copied
     * @param path Path to file or directory
     * @return Total size in bytes
     */
    virtual qint64 calculateTotalSize(const QString& path) = 0;
    
    /**
     * @brief Check if this algorithm supports pause/resume
     * @return true if pause/resume is supported
     */
    virtual bool supportsPause() const = 0;
    
    /**
     * @brief Get the name of this algorithm
     * @return Human-readable algorithm name
     */
    virtual QString getName() const = 0;
    
    /**
     * @brief Pause the current operation
     * Default implementation does nothing
     */
    virtual void pause() {}
    
    /**
     * @brief Resume a paused operation
     * Default implementation does nothing
     */
    virtual void resume() {}
    
    /**
     * @brief Cancel the current operation
     * Default implementation does nothing
     */
    virtual void cancel() {}

protected:
    /**
     * @brief Called when copying of a file starts
     * @param file File path being copied
     */
    virtual void onFileStart(const QString& file);
    
    /**
     * @brief Called when copying of a file completes
     * @param file File path that was copied
     */
    virtual void onFileComplete(const QString& file);

signals:
    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);
    
    /**
     * @brief Emitted when the algorithm is paused
     */
    void paused();
    
    /**
     * @brief Emitted when the algorithm is resumed
     */
    void resumed();
    
    /**
     * @brief Emitted when the algorithm is cancelled
     */
    void cancelled();
};

#endif // COPYALGORITHM_H 