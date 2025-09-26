#include "deviceutils.h"

#include <QDebug>

QString DeviceUtils::nameOfSize(const quint64 &size)
{
    quint64 num = size;
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    QStringList list;
    qreal fileSize(num);

    list << "B"
         << "KB"
         << "MB"
         << "GB"
         << "TB";   // should we use KiB since we use 1024 here?

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral("B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    return QString("%1 %2").arg(QString::number(fileSize, 'f', 1)).arg(unit);
}
