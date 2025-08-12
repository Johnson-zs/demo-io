#include "TaskHooks.h"
#include <unistd.h>

void TaskHooks::onTaskStart(CopyTask* task)
{
    Q_UNUSED(task)
    // Default implementation does nothing
}

void TaskHooks::onTaskPause(CopyTask* task)
{
    Q_UNUSED(task)
    // Default behavior: sync data to ensure consistency
    sync();
}

void TaskHooks::onTaskResume(CopyTask* task)
{
    Q_UNUSED(task)
    // Default implementation does nothing
}

void TaskHooks::onTaskComplete(CopyTask* task)
{
    Q_UNUSED(task)
    // Default behavior: sync data to ensure all writes are flushed
    sync();
}

void TaskHooks::onTaskError(CopyTask* task, const QString& error)
{
    Q_UNUSED(task)
    Q_UNUSED(error)
    // Default implementation does nothing
} 