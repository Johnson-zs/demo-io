#ifndef ANYTHINGSEARCHER_H
#define ANYTHINGSEARCHER_H

#include <QObject>
#include <QDBusInterface>
#include <QStringList>

class AnythingSearcher : public QObject
{
    Q_OBJECT
public:
    explicit AnythingSearcher(QObject *parent = nullptr);

    bool requestSearch(const QString &path, const QString &text);
    
    void cancelSearch();

signals:
    void searchFinished(const QString &query, const QStringList &results);
    void searchFailed(const QString &query, const QString &errorMessage);

private:
    QDBusInterface *anythingInterface { nullptr };
    QDBusPendingCallWatcher *currentRequest { nullptr };
    QString m_currentQuery;
    
private slots:
    void onRequestFinished(QDBusPendingCallWatcher *watcher);
};

#endif   // ANYTHINGSEARCHER_H
