#include "anythingsearcher.h"

#include <QDBusInterface>
#include <QDBusReply>
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
    : QObject { parent }
{
    anythingInterface = new QDBusInterface("com.deepin.anything",
                                           "/com/deepin/anything",
                                           "com.deepin.anything",
                                           QDBusConnection::systemBus(),
                                           this);
    anythingInterface->setTimeout(1000);
}

bool AnythingSearcher::requestSedarch(const QString &path, const QString &text)
{
    if (!anythingInterface->isValid())
        return false;
    if (path.isEmpty() || text.isEmpty())
        return false;

    const QDBusPendingReply<QStringList> reply = anythingInterface->asyncCallWithArgumentList("search", { path, text });
    auto results = reply.value();

    if (reply.error().type() != QDBusError::NoError) {
        qWarning() << "deepin-anything search failed:"
                   << QDBusError::errorString(reply.error().type())
                   << reply.error().message();
        return false;
    }
    results = batchExtract(results);

    return true;
}
