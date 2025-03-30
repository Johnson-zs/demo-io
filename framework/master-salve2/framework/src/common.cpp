#include "framework/common.h"
#include <QUuid>
#include <QProcess>
#include <QThread>
#include <QFile>
#include <QTextStream>

namespace Framework {
namespace Utils {

QString generateUniqueId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QDateTime now() {
    return QDateTime::currentDateTime();
}

double getCpuUsage() {
    // 简单实现，实际项目中应该使用更复杂的方法
    #if defined(Q_OS_LINUX)
        QFile file("/proc/stat");
        if (file.open(QIODevice::ReadOnly)) {
            QString line = file.readLine();
            file.close();
            
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 5) {
                static quint64 lastTotal = 0;
                static quint64 lastIdle = 0;
                
                quint64 user = parts[1].toULongLong();
                quint64 nice = parts[2].toULongLong();
                quint64 system = parts[3].toULongLong();
                quint64 idle = parts[4].toULongLong();
                
                quint64 total = user + nice + system + idle;
                quint64 diffTotal = total - lastTotal;
                quint64 diffIdle = idle - lastIdle;
                
                lastTotal = total;
                lastIdle = idle;
                
                if (diffTotal == 0) {
                    return 0.0;
                }
                
                return 100.0 * (1.0 - (double)diffIdle / diffTotal);
            }
        }
    #endif
    
    // 回退方式或其他平台
    return 0.0;
}

qint64 getAvailableMemory() {
    #if defined(Q_OS_LINUX)
        QFile file("/proc/meminfo");
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString line;
            qint64 memAvailable = 0;
            
            while (!stream.atEnd()) {
                line = stream.readLine();
                if (line.startsWith("MemAvailable:")) {
                    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                    if (parts.size() >= 2) {
                        memAvailable = parts[1].toLongLong() * 1024; // 转换为字节
                    }
                    break;
                }
            }
            
            file.close();
            return memAvailable;
        }
    #endif
    
    return 0;
}

qint64 getTotalMemory() {
    #if defined(Q_OS_LINUX)
        QFile file("/proc/meminfo");
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString line = stream.readLine();
            file.close();
            
            if (line.startsWith("MemTotal:")) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    return parts[1].toLongLong() * 1024; // 转换为字节
                }
            }
        }
    #endif
    
    return 0;
}

} // namespace Utils
} // namespace Framework 