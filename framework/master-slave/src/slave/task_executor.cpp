#include "slave.h"

TaskExecutor::TaskExecutor(const TaskMessage &task, QObject *parent)
    : QObject(parent)
    , task(task)
    , stopped(false)
{
} 