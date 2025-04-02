#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QDateTime>
#include <QString>

#include "fsevents.h"

class Logger {
public:
    static void logEvent(const FSEvent& event) {
        QString timestamp = event.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString typeStr = FSEvent::typeToString(event.type);
        
        qInfo() << QString("[%1] %2: %3")
                    .arg(timestamp)
                    .arg(typeStr)
                    .arg(event.path);
                    
        if (!event.extraInfo.isEmpty()) {
            qInfo() << "  Details:" << event.extraInfo;
        }
    }
    
    static void logInfo(const QString& message) {
        qInfo() << message;
    }
    
    static void logError(const QString& message) {
        qCritical() << message;
    }
};

#endif // LOGGER_H 