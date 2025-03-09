#include "anythingsearcher.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QDebug>

namespace {
QString extract(const QString &pathWithMetadata)
{
    const qsizetype pos = pathWithMetadata.indexOf("<\\>");
    return (pos != -1) ? pathWithMetadata.left(pos) : pathWithMetadata;
}

QStringList batchExtract(const QStringList &paths)
{
    QStringList result;
    result.reserve(paths.size());

    std::transform(paths.cbegin(), paths.cend(),
                   std::back_inserter(result),
                   [](const QString &path) { return extract(path); });

    return result;
}
}   // namespace

AnythingSearcher::AnythingSearcher(QObject *parent)
    : QObject { parent }, currentRequest(nullptr)
{
    anythingInterface = new QDBusInterface("com.deepin.anything",
                                           "/com/deepin/anything",
                                           "com.deepin.anything",
                                           QDBusConnection::systemBus(),
                                           this);
    anythingInterface->setTimeout(1000);
}

bool AnythingSearcher::requestSearch(const QString &path, const QString &text)
{
    if (!anythingInterface->isValid())
        return false;
    if (path.isEmpty() || text.isEmpty())
        return false;

    // 取消先前的请求（如果有）
    cancelSearch();

    // 保存当前查询
    m_currentQuery = text;

    // 使用异步调用
    QDBusPendingCall pendingCall = anythingInterface->asyncCallWithArgumentList("search", { path, text });
    currentRequest = new QDBusPendingCallWatcher(pendingCall, this);
    
    connect(currentRequest, &QDBusPendingCallWatcher::finished,
            this, &AnythingSearcher::onRequestFinished);
    
    return true;
}

void AnythingSearcher::cancelSearch()
{
    if (currentRequest) {
        disconnect(currentRequest, nullptr, this, nullptr);
        delete currentRequest;
        currentRequest = nullptr;
        m_currentQuery.clear();  // 清除当前查询
    }
}

void AnythingSearcher::onRequestFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QStringList> reply = *watcher;
    
    if (reply.isError()) {
        emit searchFailed(m_currentQuery,  // 使用保存的查询
            QString("搜索失败: %1").arg(reply.error().message()));
    } else {
        QStringList results = batchExtract(reply.value());
        emit searchFinished(m_currentQuery, results);  // 使用保存的查询
    }
    
    // 清理
    watcher->deleteLater();
    currentRequest = nullptr;
}
