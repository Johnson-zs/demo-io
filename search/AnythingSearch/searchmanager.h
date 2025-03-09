#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "anythingsearcher.h"

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <QHash>

class SearchManager : public QObject
{
    Q_OBJECT
public:
    explicit SearchManager(QObject *parent = nullptr);
    
    // 处理用户输入
    void processUserInput(const QString &searchPath, const QString &searchText);
    
    // 清除缓存
    void clearCache();

signals:
    // 搜索结果信号
    void searchResultsReady(const QStringList &results);
    void searchError(const QString &errorMessage);

private slots:
    void executeSearch();
    void onSearchFinished(const QString &query, const QStringList &results);
    void onSearchFailed(const QString &query, const QString &errorMessage);

private:
    // 输入变化类型
    enum class InputChangeType { Addition, Deletion, Replacement, Unknown };
    
    // 分析输入变化
    InputChangeType analyzeInputChange(const QString &oldText, const QString &newText);
    
    // 在本地过滤结果
    QStringList filterLocalResults(const QStringList &sourceResults, const QString &query);
    
    // 确定防抖延迟
    int determineDebounceDelay(const QString &text);
    
    // 判断是否需要延迟搜索
    bool shouldDelaySearch(const QString &text);

    AnythingSearcher *m_searcher;
    QTimer m_debounceTimer;
    QString m_pendingSearchPath;
    QString m_pendingSearchText;
    QString m_lastSearchText;
    QDateTime m_lastSearchTime;
    
    // 缓存结构
    QMap<QString, QStringList> m_resultsCache;
    int m_throttleInterval = 300; // 毫秒
    
    // 当前搜索工作目录
    QString m_currentSearchPath;

    // 添加文件名缓存
    QHash<QString, QString> m_fileNameCache;  // 路径 -> 文件名的映射
};

#endif // SEARCHMANAGER_H 
