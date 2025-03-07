#ifndef ANYTHINGSEARCHER_H
#define ANYTHINGSEARCHER_H

#include <QObject>
#include <QDBusInterface>

class AnythingSearcher : public QObject
{
    Q_OBJECT
public:
    explicit AnythingSearcher(QObject *parent = nullptr);

    bool requestSedarch(const QString &path, const QString &text);

signals:

private:
    QDBusInterface *anythingInterface { nullptr };
};

#endif   // ANYTHINGSEARCHER_H
