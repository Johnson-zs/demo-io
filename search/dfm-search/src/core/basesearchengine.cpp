#include "dfm-search/basesearchengine.h"

namespace DFM {
namespace Search {

BaseSearchEngine::BaseSearchEngine(QObject* parent)
    : ISearchEngine(parent)
    , m_state(SearchState::Idle)
{
}

void BaseSearchEngine::pauseSearch()
{
    if (m_state == SearchState::Searching) {
        setState(SearchState::Paused);
    }
}

void BaseSearchEngine::resumeSearch()
{
    if (m_state == SearchState::Paused) {
        setState(SearchState::Searching);
    }
}

void BaseSearchEngine::cancelSearch()
{
    if (m_state == SearchState::Searching || m_state == SearchState::Paused) {
        setState(SearchState::Cancelled);
    }
}

SearchState BaseSearchEngine::state() const
{
    return m_state;
}

QString BaseSearchEngine::lastError() const
{
    return m_lastError;
}

void BaseSearchEngine::setState(SearchState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
        
        if (state == SearchState::Completed) {
            emit searchCompleted(true);
        } else if (state == SearchState::Error) {
            emit searchCompleted(false);
        }
    }
}

void BaseSearchEngine::setError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
    setState(SearchState::Error);
}

void BaseSearchEngine::updateProgress(const SearchProgress& progress)
{
    emit progressChanged(progress);
}

void BaseSearchEngine::addResults(const SearchResultSet& results)
{
    emit resultsReady(results);
}

} // namespace Search
} // namespace DFM 
