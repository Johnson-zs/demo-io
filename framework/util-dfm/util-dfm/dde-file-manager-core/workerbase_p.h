#ifndef DFM_WORKERBASE_P_H
#define DFM_WORKERBASE_P_H

#include "workerbase.h"
#include <QString>
#include <QByteArray>
#include <QMap>

namespace DFM {

class WorkerBridge;  // 前向声明

/**
 * @class WorkerResultPrivate
 * @brief WorkerResult 的私有实现
 */
class WorkerResultPrivate
{
public:
    bool success = false;
    int error = 0;
    QString errorString;
};

/**
 * @class WorkerBasePrivate
 * @brief WorkerBase 的私有实现
 */
class WorkerBasePrivate
{
public:
    QByteArray protocol;
    QByteArray poolSocket;
    QByteArray appSocket;
    MetaData metaData;
    QMap<QString, QVariant> config;
    bool killed = false;
};

} // namespace DFM

#endif // DFM_WORKERBASE_P_H 