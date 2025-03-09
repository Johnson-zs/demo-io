#ifndef MOCKSEARCHER_H
#define MOCKSEARCHER_H

#include "searcherinterface.h"

#include <QTimer>
#include <QDir>
#include <QFileInfoList>
#include <QFuture>
#include <QtConcurrent>

class MockSearcher : public SearcherInterface
{
    Q_OBJECT
public:
    explicit MockSearcher(QObject *parent = nullptr);
    ~MockSearcher();

    bool requestSearch(const QString &path, const QString &text) override;
    void cancelSearch() override;

    // 设置模拟延迟（毫秒）
    void setSimulatedDelay(int msec);
    
    // 设置是否模拟错误
    void setSimulateErrors(bool simulate);
    
    // 设置模拟的文件数量上限
    void setMaxFileCount(int count);

private:
    // 执行实际搜索
    void performSearch(const QString &path, const QString &text);
    
    // 递归扫描目录
    QStringList scanDirectory(const QString &path, const QString &pattern, int maxDepth = 5);
    
    // 格式化结果为 Anything 格式
    QString formatResult(const QString &path);

    QTimer *m_delayTimer;
    QString m_currentQuery;
    QString m_currentPath;
    bool m_isCancelled;
    bool m_simulateErrors;
    int m_simulatedDelay;
    int m_maxFileCount;
    QFuture<void> m_searchFuture;
};

#endif // MOCKSEARCHER_H 