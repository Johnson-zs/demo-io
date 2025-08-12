#ifndef TASKHOOKS_H
#define TASKHOOKS_H

#include <QString>

// Forward declaration
class CopyTask;

/**
 * @brief Interface for task lifecycle hooks
 * 
 * This interface allows developers to inject custom behavior at key points
 * during the copy task lifecycle, such as when tasks start, pause, resume,
 * complete, or encounter errors.
 */
class TaskHooks
{
public:
    virtual ~TaskHooks() = default;
    
    /**
     * @brief Called when a copy task starts
     * @param task The task that is starting
     */
    virtual void onTaskStart(CopyTask* task);
    
    /**
     * @brief Called when a copy task is paused
     * @param task The task that was paused
     * 
     * Default behavior: sync data to ensure consistency
     */
    virtual void onTaskPause(CopyTask* task);
    
    /**
     * @brief Called when a copy task resumes from pause
     * @param task The task that is resuming
     */
    virtual void onTaskResume(CopyTask* task);
    
    /**
     * @brief Called when a copy task completes successfully
     * @param task The task that completed
     * 
     * Default behavior: sync data to ensure all writes are flushed
     */
    virtual void onTaskComplete(CopyTask* task);
    
    /**
     * @brief Called when a copy task encounters an error
     * @param task The task that encountered an error
     * @param error Error message describing what went wrong
     */
    virtual void onTaskError(CopyTask* task, const QString& error);
};

#endif // TASKHOOKS_H 