#ifndef FSEVENTS_H
#define FSEVENTS_H

#include <QString>
#include <QDateTime>

struct FSEvent {
    enum EventType {
        Create = 1,
        Delete = 2,
        Modify = 3,
        Unknown = 0
    };

    EventType type;
    QString path;
    QString extraInfo;
    quint32 cookie;
    quint16 major;
    quint8 minor;
    QDateTime timestamp;

    static QString typeToString(EventType type) {
        switch (type) {
        case Create: return "Created";
        case Delete: return "Deleted";
        case Modify: return "Modified";
        default: return "Unknown";
        }
    }
};

#endif // FSEVENTS_H 