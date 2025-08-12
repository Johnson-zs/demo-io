#include "CopyAlgorithm.h"

CopyAlgorithm::CopyAlgorithm(QObject *parent)
    : QObject(parent)
{
}

void CopyAlgorithm::onFileStart(const QString& file)
{
    Q_UNUSED(file)
    // Default implementation does nothing
    // Subclasses can override for custom behavior
}

void CopyAlgorithm::onFileComplete(const QString& file)
{
    Q_UNUSED(file)
    // Default implementation does nothing
    // Subclasses can override for custom behavior
}
