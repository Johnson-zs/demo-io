#include "abstractsearchengine.h"

namespace DFM6 {
namespace Search {

AbstractSearchEngine::AbstractSearchEngine(QObject *parent)
    : QObject(parent),
      m_status(SearchStatus::Ready),
      m_cancelled(false)
{
}

AbstractSearchEngine::~AbstractSearchEngine()
{
}

void AbstractSearchEngine::setStatus(SearchStatus status)
{
    m_status.store(status);
    emit statusChanged(status);
}

void AbstractSearchEngine::reportProgress(int current, int total)
{
    emit progressChanged(current, total);
}

void AbstractSearchEngine::reportError(const QString &message)
{
    emit error(message);
}

}  // namespace Search
}  // namespace DFM6 