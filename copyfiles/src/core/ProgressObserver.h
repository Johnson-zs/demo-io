#ifndef PROGRESSOBSERVER_H
#define PROGRESSOBSERVER_H

#include <QString>
#include <QtGlobal>

/**
 * @brief Interface for observing copy operation progress
 * 
 * This interface allows components to receive notifications about
 * the progress of file copy operations, including start/completion
 * of individual files, overall progress, and error conditions.
 * 
 * It also provides state query methods for algorithms to check
 * if they should pause or stop execution.
 */
class ProgressObserver
{
public:
    virtual ~ProgressObserver() = default;
    
    /**
     * @brief Called when overall progress is updated
     * @param current Number of bytes copied so far
     * @param total Total number of bytes to copy
     */
    virtual void onProgressUpdate(qint64 current, qint64 total) = 0;
    
    /**
     * @brief Called when copying of a file starts
     * @param filename Name of the file being copied
     */
    virtual void onFileStart(const QString& filename) = 0;
    
    /**
     * @brief Called when copying of a file completes successfully
     * @param filename Name of the file that was copied
     */
    virtual void onFileComplete(const QString& filename) = 0;
    
    /**
     * @brief Called when an error occurs during copying
     * @param error Error message describing what went wrong
     */
    virtual void onError(const QString& error) = 0;
    
    /**
     * @brief Called when the entire copy operation completes successfully
     */
    virtual void onComplete() = 0;
    
    /**
     * @brief Query methods for algorithms to check execution state
     * These methods allow algorithms to be responsive to pause/stop requests
     */
    virtual bool shouldPause() const = 0;
    virtual bool shouldStop() const = 0;
    
    /**
     * @brief Wait while paused using condition variable (efficient blocking)
     * This method blocks the calling thread until resume or stop is requested
     */
    virtual void waitWhilePaused() = 0;
};

#endif // PROGRESSOBSERVER_H 