#include "searchmanager.h"

#include <QFileInfo>
#include <QtConcurrent>
#include <QRegularExpression>
#include <QDebug>

SearchManager::SearchManager(QObject *parent)
    : QObject(parent), m_searcher(nullptr), m_ownsSearcher(false)
{
    // 初始化防抖定时器
    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout, this,[this]() {
        qDebug() << "===> timer search:" << m_pendingSearchText;
        executeSearch();
    });

    // 初始化时间戳
    m_lastSearchTime = QDateTime::currentDateTime();
}

SearchManager::SearchManager(SearcherInterface *searcher, QObject *parent)
    : SearchManager(parent)
{
    setSearcher(searcher);
}

void SearchManager::setSearcher(SearcherInterface *searcher)
{
    // 清理旧的搜索器
    if (m_searcher && m_ownsSearcher) {
        delete m_searcher;
    }
    
    m_searcher = searcher;
    m_ownsSearcher = false;
    
    if (m_searcher) {
        // 连接搜索信号
        connect(m_searcher, &SearcherInterface::searchFinished,
                this, &SearchManager::onSearchFinished);
        connect(m_searcher, &SearcherInterface::searchFailed,
                this, &SearchManager::onSearchFailed);
    }
}

void SearchManager::processUserInput(const QString &searchPath, const QString &searchText)
{
    qDebug() << "===> input: " << searchText;

    // 更新路径
    if (m_currentSearchPath != searchPath) {
        qDebug() << "===> path changed: " << searchPath;
        m_currentSearchPath = searchPath;
        m_lastSearchText.clear();
        m_resultsCache.clear();
    }

    m_pendingSearchPath = searchPath;
    m_pendingSearchText = searchText;

    // 如果输入为空，直接清空结果
    if (searchText.isEmpty()) {
        emit searchResultsReady(QStringList());
        return;
    }

    // 尝试从缓存获取结果
    QStringList cachedResults;
    if (!m_lastSearchText.isEmpty() && tryGetFromCache(searchText, cachedResults)) {
        emit searchResultsReady(cachedResults);
        m_lastSearchText = searchText;
        return;
    }

    // 应用防抖：重置定时器
    m_debounceTimer.start(determineDebounceDelay(searchText));

    // 应用节流：检查上次搜索时间
    QDateTime now = QDateTime::currentDateTime();
    if (m_lastSearchTime.msecsTo(now) >= m_throttleInterval) {
        // 如果间隔足够且不是应该延迟的搜索，则立即执行
        if (!shouldDelaySearch(searchText)) {
            qDebug() << "===> direct search:" << searchText;
            m_debounceTimer.stop();
            executeSearch();
        }
    }
}

void SearchManager::executeSearch()
{
    if (m_pendingSearchText.isEmpty()) {
        emit searchResultsReady(QStringList());
        return;
    }

    // 检查缓存
    if (m_resultsCache.contains(m_pendingSearchText)) {
        emit searchResultsReady(m_resultsCache[m_pendingSearchText]);
        m_lastSearchText = m_pendingSearchText;
        return;
    }

    qDebug() << "[excute] About to search: " << m_pendingSearchText;
    
    // 检查搜索器是否存在
    if (!m_searcher) {
        emit searchError("搜索器未初始化");
        return;
    }
    
    // 执行实际搜索
    if (!m_searcher->requestSearch(m_pendingSearchPath, m_pendingSearchText)) {
        emit searchError("搜索请求失败");
    }

    // 更新最后搜索时间
    m_lastSearchTime = QDateTime::currentDateTime();
    m_lastSearchText = m_pendingSearchText;
}

void SearchManager::onSearchFinished(const QString &query, const QStringList &results)
{
    // 仅处理最新的查询结果
    if (query == m_pendingSearchText) {
        m_resultsCache[query] = results;
        emit searchResultsReady(results);
    }
}

void SearchManager::onSearchFailed(const QString &query, const QString &errorMessage)
{
    if (query == m_pendingSearchText) {
        emit searchError(errorMessage);
    }
}

SearchManager::InputChangeType SearchManager::analyzeInputChange(const QString &oldText, const QString &newText)
{
    if (newText.length() > oldText.length()) {
        if (newText.startsWith(oldText)) {
            return InputChangeType::Addition;
        }
    } else if (newText.length() < oldText.length()) {
        if (oldText.startsWith(newText)) {
            return InputChangeType::Deletion;
        }
    }

    return InputChangeType::Replacement;
}

QString SearchManager::getFileName(const QString &filePath)
{
    
    // 缓存未命中，计算文件名
    int lastSeparator = filePath.lastIndexOf('/');
    QString fileName = (lastSeparator == -1) ? 
        filePath : filePath.mid(lastSeparator + 1);

    return fileName;
}

QStringList SearchManager::filterLocalResults(const QStringList &sourceResults, const QString &query)
{
    QStringList filteredResults;
    filteredResults.reserve(sourceResults.size());
    
    const QString queryLower = query.toLower();
    
    for (const QString &filePath : sourceResults) {
        if (getFileName(filePath).toLower().contains(queryLower)) {
            filteredResults.append(filePath);
        }
    }
    
    return filteredResults;
}

int SearchManager::determineDebounceDelay(const QString &text)
{
    // 基础等待时间
    int delay = 200; // 毫秒

    // 针对短输入增加延迟
    if (text.length() <= 2) {
        delay += 150;
    }

    // 对于可能返回大量结果的特殊字符增加延迟
    if (text.contains('*') || text.contains('?') || text.startsWith('.')) {
        delay += 200;
    }

    return delay;
}

bool SearchManager::shouldDelaySearch(const QString &text)
{
    // 对于过短或者通配符搜索，应该延迟
    return text.length() < 2 || text == "." || text == "*";
}

void SearchManager::clearCache()
{
    m_resultsCache.clear();
    m_lastSearchText.clear();
}

bool SearchManager::tryGetFromCache(const QString &searchText, QStringList &results)
{
    // 首先检查是否直接有缓存
    if (m_resultsCache.contains(searchText)) {
        qDebug() << "===> search from direct cache: " << searchText;
        results = m_resultsCache[searchText];
        return true;
    }

    // 检查输入变化类型
    InputChangeType changeType = analyzeInputChange(m_lastSearchText, searchText);
    
    // 根据变化类型处理
    switch (changeType) {
    case InputChangeType::Addition:
        return handleIncrementalSearch(searchText, results);
    case InputChangeType::Deletion:
        return handleDeletionSearch(searchText, results);
    default:
        qDebug() << "==> cache break: replacement or unknown change";
        return false;
    }
}

bool SearchManager::handleIncrementalSearch(const QString &searchText, QStringList &results)
{
    // 如果新输入是旧输入的扩展，并且我们有旧缓存
    if (searchText.startsWith(m_lastSearchText) && m_resultsCache.contains(m_lastSearchText)) {
        qDebug() << "===> search from cache(add): " << searchText;
        results = filterLocalResults(m_resultsCache[m_lastSearchText], searchText);
        m_resultsCache[searchText] = results;
        return true;
    }
    return false;
}

bool SearchManager::handleDeletionSearch(const QString &searchText, QStringList &results)
{
    // 查找最佳前缀匹配
    QString bestPrefix;
    
    for (auto it = m_resultsCache.begin(); it != m_resultsCache.end(); ++it) {
        // 找到所有是当前查询前缀的缓存项
        if (searchText.startsWith(it.key())) {
            // 选择最长的前缀（最接近当前查询的）
            if (bestPrefix.isEmpty() || it.key().length() > bestPrefix.length()) {
                bestPrefix = it.key();
            }
        }
    }
    
    if (!bestPrefix.isEmpty()) {
        qDebug() << "===> search from prefix cache(del): " << bestPrefix << " for " << searchText;
        results = filterLocalResults(m_resultsCache[bestPrefix], searchText);
        m_resultsCache[searchText] = results;
        return true;
    }
    
    return false;
}
