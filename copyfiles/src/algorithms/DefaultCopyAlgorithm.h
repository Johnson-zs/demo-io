#ifndef DEFAULTCOPYALGORITHM_H
#define DEFAULTCOPYALGORITHM_H

#include "core/CopyAlgorithm.h"
#include <QAtomicInt>
#include <QMutex>

/**
 * @brief Default copy algorithm implementation
 * 
 * This algorithm uses a two-tier approach:
 * 1. Primary: copy_file_range() system call for efficient kernel-space copying
 * 2. Fallback: Traditional read()/write() with 512KB chunks
 * 
 * The algorithm supports pause/resume functionality and provides detailed
 * progress reporting.
 */
class DefaultCopyAlgorithm : public CopyAlgorithm
{
    Q_OBJECT

public:
    explicit DefaultCopyAlgorithm(QObject* parent = nullptr);
    virtual ~DefaultCopyAlgorithm() = default;

    // CopyAlgorithm interface implementation
    bool copyFile(const QString& source, const QString& dest, 
                  ProgressObserver* observer = nullptr) override;
    bool copyDirectory(const QString& source, const QString& dest,
                      ProgressObserver* observer = nullptr) override;
    qint64 calculateTotalSize(const QString& path) override;
    bool supportsPause() const override;
    QString getName() const override;
    
    void pause() override;
    void resume() override;
    void cancel() override;

private:
    static constexpr qint64 CHUNK_SIZE = 512 * 1024; // 512KB chunks
    static constexpr int MAX_RETRIES = 3;
    
    // Core copy methods
    bool copyFileRange(const QString& source, const QString& dest, 
                      ProgressObserver* observer = nullptr);
    bool copyFileChunked(const QString& source, const QString& dest,
                        ProgressObserver* observer = nullptr);
    
    // Internal copyFile method for directory copying (doesn't reset progress)
    bool copyFileInternal(const QString& source, const QString& dest,
                         ProgressObserver* observer = nullptr);
    
    // Helper methods
    bool shouldFallback(int errorCode) const;
    bool copyDirectoryRecursive(const QString& source, const QString& dest,
                               ProgressObserver* observer = nullptr);
    bool createDirectoryStructure(const QString& source, const QString& dest);
    void waitIfPaused();
    bool isCancelled() const;
    
    // No state management needed - handled by CopyWorker
    
    // Progress tracking
    qint64 m_totalBytes;
    qint64 m_copiedBytes;
    bool m_isDirectoryCopy; // Flag to distinguish directory vs single file copy
};

#endif // DEFAULTCOPYALGORITHM_H 