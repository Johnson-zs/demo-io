#pragma once
#include <QString>
#include <QDataStream>

struct Task {
    int taskId;
    QString command;
    
    friend QDataStream &operator<<(QDataStream &out, const Task &task) {
        out << task.taskId << task.command;
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, Task &task) {
        in >> task.taskId >> task.command;
        return in;
    }
};

struct Result {
    int taskId;
    QString result;
    
    friend QDataStream &operator<<(QDataStream &out, const Result &result) {
        out << result.taskId << result.result;
        return out;
    }
    
    friend QDataStream &operator>>(QDataStream &in, Result &result) {
        in >> result.taskId >> result.result;
        return in;
    }
}; 