#ifndef SYNCCOPYALGORITHM_H
#define SYNCCOPYALGORITHM_H

#include "../core/CopyAlgorithm.h"
#include "../core/ProgressObserver.h"
#include <QAtomicInt>
#include <QMutex>

/**
 * @brief Windows-style sync copy algorithm with no kernel caching
 *
 * This algorithm implements Windows "Removal policy" behavior by:
 * - Using O_SYNC flag to bypass kernel caching
 * - Calling syncfs() during pause operations
 * - Supporting pause/resume with USB device removal
 * - Dynamic chunk size optimization based on file size
 */
class SyncCopyAlgorithm : public CopyAlgorithm
{
    Q_OBJECT

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
    // Constants for chunk size strategy
    static constexpr qint64 SMALL_FILE_CHUNK_SIZE = 1024 * 1024;   // for small files
    static constexpr qint64 LARGE_FILE_CHUNK_SIZE = 1 * 1024 * 1024;   // for large files
    static constexpr qint64 LARGE_FILE_THRESHOLD = 16 * 1024 * 1024;   //  threshold
    static constexpr int MAX_RETRIES = 3;

    /**
     * @brief Strategy pattern for chunk size determination
     */
    struct ChunkStrategy
    {
        static qint64 getOptimalChunkSize(qint64 fileSize)
        {
            return fileSize >= LARGE_FILE_THRESHOLD ? LARGE_FILE_CHUNK_SIZE : SMALL_FILE_CHUNK_SIZE;
        }
    };

    /**
     * @brief File writer strategies following Strategy pattern
     */
    enum class WriteMode {
        Direct,    // O_DIRECT mode
        Normal     // Normal mode (no special flags)
    };

    struct FileWriter {
        int fd;
        WriteMode mode;
        size_t alignment;
        
        FileWriter(int fd, WriteMode mode, size_t alignment = 4096) 
            : fd(fd), mode(mode), alignment(alignment) {}
    };

    // Core copy methods
    bool copyFileRange(const QString &source, const QString &dest,
                       ProgressObserver *observer = nullptr);
    bool copyFileChunked(const QString &source, const QString &dest,
                         ProgressObserver *observer = nullptr);

    // Internal copyFile method for directory copying (doesn't reset progress)
    bool copyFileInternal(const QString &source, const QString &dest,
                          ProgressObserver *observer = nullptr);

    // Helper methods following Single Responsibility Principle
    bool shouldFallback(int errorCode) const;
    bool copyDirectoryRecursive(const QString &source, const QString &dest,
                                ProgressObserver *observer = nullptr);
    bool createDirectoryStructure(const QString &source, const QString &dest);
    void performSyncOperation(int fd, const QString &path) const;
    qint64 getFileSize(const QString &filePath) const;
    void waitIfPaused();
    bool isCancelled() const;

    // New helper methods for SOLID refactoring
    FileWriter openDestinationFile(const QString &dest, WriteMode preferredMode);
    FileWriter reopenDestinationFileForResume(const QString &dest, WriteMode preferredMode);
    bool copyWithWriter(int srcFd, FileWriter writer, qint64 totalSize, 
                       const QString &dest, ProgressObserver *observer, bool &directModeSwitched);
    char* allocateAlignedBuffer(size_t size, size_t alignment);
    bool handlePauseResume(FileWriter &writer, const QString &dest, ProgressObserver *observer);

    // Progress tracking
    qint64 m_totalBytes;
    qint64 m_copiedBytes;
    bool m_isDirectoryCopy;   // Flag to distinguish directory vs single file copy
};

#endif   // SYNCCOPYALGORITHM_H
