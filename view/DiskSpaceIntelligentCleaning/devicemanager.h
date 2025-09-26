#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QVariantMap>

class DeviceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DeviceManager)

public:
    static DeviceManager &instance();

    QStringList diskIdList() const;
    QVariantMap diskInfo(const QString &id);
    QString idLabel(const QString &id);
    QString mountPoint(const QString &id);
    qint64 sizeTotal(const QString &id);
    qint64 sizeUsed(const QString &id);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    void fetchData();
    bool isDisplayDevice(const QVariantMap &datas);

private:
    QMap<QString, QVariantMap> m_diskDatas;
};

#endif   // DEVICEMANAGER_H
