#include "qsearch/engines/search_engine.h"

namespace QSearch {

SearchEngine::SearchEngine(QObject* parent) : QObject(parent) {
}

SearchEngine::~SearchEngine() {
}

QString SearchEngine::errorString() const {
    return m_errorString;
}

void SearchEngine::setErrorString(const QString& errorString) {
    m_errorString = errorString;
    emit error(m_errorString);
}

} // namespace QSearch 