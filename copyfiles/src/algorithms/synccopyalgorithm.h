#ifndef SYNCCOPYALGORITHM_H
#define SYNCCOPYALGORITHM_H

#include "core/CopyAlgorithm.h"
#include <QAtomicInt>
#include <QMutex>

class SyncCopyAlgorithm : public CopyAlgorithm
{
public:
    explicit SyncCopyAlgorithm(QObject *parent = nullptr);
    virtual ~SyncCopyAlgorithm() = default;

    // CopyAlgorithm interface implementation
    bool copyFile(const QString &source, const QString &dest,
                  ProgressObserver *observer = nullptr) override;
    bool copyDirectory(const QString &source, const QString &dest,
                       ProgressObserver *observer = nullptr) override;
    qint64 calculateTotalSize(const QString &path) override;
    bool supportsPause() const override;
    QString getName() const override;

    void pause() override;
    void resume() override;
    void cancel() override;

private:
    static constexpr qint64 CHUNK_SIZE = 512 * 1024;   // 512KB chunks
    static constexpr int MAX_RETRIES = 3;

    // Core copy methods
    bool copyFileRange(const QString &source, const QString &dest,
                       ProgressObserver *observer = nullptr);
    bool copyFileChunked(const QString &source, const QString &dest,
                         ProgressObserver *observer = nullptr);

    // Internal copyFile method for directory copying (doesn't reset progress)
    bool copyFileInternal(const QString &source, const QString &dest,
                          ProgressObserver *observer = nullptr);

    // Helper methods
    bool shouldFallback(int errorCode) const;
    bool copyDirectoryRecursive(const QString &source, const QString &dest,
                                ProgressObserver *observer = nullptr);
    bool createDirectoryStructure(const QString &source, const QString &dest);
    void waitIfPaused();
    bool isCancelled() const;

    // No state management needed - handled by CopyWorker

    // Progress tracking
    qint64 m_totalBytes;
    qint64 m_copiedBytes;
    bool m_isDirectoryCopy;   // Flag to distinguish directory vs single file copy
};

#endif   // SYNCCOPYALGORITHM_H
