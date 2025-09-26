#include "devicemanager.h"

#include <QDBusInterface>
#include <QDBusReply>

DeviceManager &DeviceManager::instance()
{
    static DeviceManager ins;
    return ins;
}

QStringList DeviceManager::diskIdList() const
{
    return m_diskDatas.keys();
}

QVariantMap DeviceManager::diskInfo(const QString &id)
{
    return m_diskDatas.value(id);
}

QString DeviceManager::idLabel(const QString &id)
{
    return diskInfo(id).value("IdLabel").toString();
}

QString DeviceManager::mountPoint(const QString &id)
{
    return diskInfo(id).value("MountPoint").toString();
}

qint64 DeviceManager::sizeTotal(const QString &id)
{
    return diskInfo(id).value("SizeTotal").toULongLong();
}

qint64 DeviceManager::sizeUsed(const QString &id)
{
    return diskInfo(id).value("SizeUsed").toULongLong();
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject { parent }
{
    // 数据初始化
    fetchData();
}

void DeviceManager::fetchData()
{
    QDBusInterface *interface = new QDBusInterface("org.deepin.Filemanager.Daemon",
                                                   "/org/deepin/Filemanager/Daemon/DeviceManager",
                                                   "org.deepin.Filemanager.Daemon.DeviceManager",
                                                   QDBusConnection::sessionBus(),
                                                   this);
    if (!interface->isValid()) {
        qWarning() << "dbus init failed";
        return;
    }

    // 获取设备id
    QDBusReply<QStringList> reply = interface->call("GetBlockDevicesIdList", 2);
    if (!reply.isValid()) {
        qWarning() << "call dbus faield: " << reply.error().message();
        return;
    }

    // 获取设备数据
    const QStringList &ids = reply.value();
    for (const auto &id : ids) {
        QDBusReply<QVariantMap> reply = interface->call("QueryBlockDeviceInfo", id, false);
        if (!reply.isValid())
            continue;
        const QVariantMap &values = reply;
        if (!isDisplayDevice(values))
            continue;

        m_diskDatas.insert(values.value("Id").toString(), values);
    }
}

// 筛选块设备
bool DeviceManager::isDisplayDevice(const QVariantMap &datas)
{
    bool hintIgnore { qvariant_cast<bool>(datas.value("HintIgnore")) };
    bool hasFileSystem { qvariant_cast<bool>(datas.value("HasFileSystem")) };
    if (!hasFileSystem || hintIgnore)
        return false;

    return true;
}
