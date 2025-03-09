#ifndef SEARCHERINTERFACE_H
#define SEARCHERINTERFACE_H

#include <QObject>
#include <QStringList>

class SearcherInterface : public QObject
{
    Q_OBJECT
public:
    explicit SearcherInterface(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~SearcherInterface() {}

    // 搜索接口
    virtual bool requestSearch(const QString &path, const QString &text) = 0;
    
    // 取消搜索
    virtual void cancelSearch() = 0;

signals:
    // 搜索结果信号
    void searchFinished(const QString &query, const QStringList &results);
    void searchFailed(const QString &query, const QString &errorMessage);
};

#endif // SEARCHERINTERFACE_H 