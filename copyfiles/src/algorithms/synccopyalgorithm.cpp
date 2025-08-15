#include "synccopyalgorithm.h"

#include "../core/FileSystemUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <fcntl.h>
#include <qlogging.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>

// Linux-specific copy_file_range system call
#ifdef __linux__
#    include <sys/syscall.h>
#    ifndef __NR_copy_file_range
#        define __NR_copy_file_range 326
#    endif

// Use the system copy_file_range if available, otherwise use syscall
#    ifndef _GNU_SOURCE
static ssize_t my_copy_file_range(int fd_in, off_t *off_in, int fd_out,
                                  off_t *off_out, size_t len, unsigned int flags)
{
    return syscall(__NR_copy_file_range, fd_in, off_in, fd_out, off_out, len, flags);
}
#        define copy_file_range my_copy_file_range
#    endif
#endif

SyncCopyAlgorithm::SyncCopyAlgorithm(QObject *parent)
    : CopyAlgorithm(parent), m_totalBytes(0), m_copiedBytes(0), m_isDirectoryCopy(false)
{
}

bool SyncCopyAlgorithm::copyFile(const QString &source, const QString &dest,
                                 ProgressObserver *observer)
{
    if (observer && observer->shouldStop()) {
        return false;
    }

    // If not in directory copy mode, reset progress tracking for single file
    if (!m_isDirectoryCopy) {
        m_totalBytes = calculateTotalSize(source);
        m_copiedBytes = 0;
    }

    auto result = copyFileInternal(source, dest, observer);

    // Ensure data is synced to disk after file copy completion
    if (result) {
        int destFd = open(dest.toLocal8Bit().constData(), O_RDONLY);
        if (destFd >= 0) {
            performSyncOperation(destFd, dest);
            close(destFd);
        }
    }

    return result;
}

bool SyncCopyAlgorithm::copyFileInternal(const QString &source, const QString &dest,
                                         ProgressObserver *observer)
{
    if (observer && observer->shouldStop()) {
        return false;
    }

    // Notify observer that file copy is starting
    if (observer) {
        observer->onFileStart(source);
    }

    bool success = false;

// Try copy_file_range first
#ifdef __linux__
    success = copyFileRange(source, dest, observer);
#endif

    // Fallback to chunked copy if copy_file_range failed
    if (!success && (!observer || !observer->shouldStop())) {
        success = copyFileChunked(source, dest, observer);
    }

    // Notify observer of completion
    if (observer) {
        if (success) {
            observer->onFileComplete(source);
        } else {
            observer->onError(QString("Failed to copy file: %1 to %2").arg(source).arg(dest));
        }
    }

    return success;
}

bool SyncCopyAlgorithm::copyDirectory(const QString &source, const QString &dest,
                                      ProgressObserver *observer)
{
    if (observer && observer->shouldStop()) {
        return false;
    }

    // Set directory copy mode
    m_isDirectoryCopy = true;

    // Calculate total size for progress tracking
    m_totalBytes = calculateTotalSize(source);
    m_copiedBytes = 0;

    if (m_totalBytes < 0) {
        if (observer) {
            observer->onError("Failed to calculate directory size");
        }
        m_isDirectoryCopy = false;
        return false;
    }

    // Create destination directory structure
    if (!createDirectoryStructure(source, dest)) {
        if (observer) {
            observer->onError("Failed to create directory structure");
        }
        m_isDirectoryCopy = false;
        return false;
    }

    // Copy all files recursively
    bool result = copyDirectoryRecursive(source, dest, observer);

    // Windows-style sync for directory: ensure all data is written to disk
    int destFd = open(dest.toLocal8Bit().constData(), O_RDONLY);
    if (destFd >= 0) {
        performSyncOperation(destFd, dest);
        close(destFd);
    }

    // Reset directory copy mode
    m_isDirectoryCopy = false;

    return result;
}

qint64 SyncCopyAlgorithm::calculateTotalSize(const QString &path)
{
    return FileSystemUtils::calculateTotalSize(path);
}

bool SyncCopyAlgorithm::supportsPause() const
{
    return true;
}

QString SyncCopyAlgorithm::getName() const
{
    return "Windows-style Sync Copy (No Cache, Dynamic Chunks)";
}

void SyncCopyAlgorithm::pause()
{
    // Pause/resume is now handled by CopyWorker
    emit paused();
}

void SyncCopyAlgorithm::resume()
{
    // Pause/resume is now handled by CopyWorker
    emit resumed();
}

void SyncCopyAlgorithm::cancel()
{
    // Cancel is now handled by CopyWorker
    emit cancelled();
}

bool SyncCopyAlgorithm::copyFileRange(const QString &source, const QString &dest,
                                      ProgressObserver *observer)
{
#ifdef __linux__
    int srcFd = open(source.toLocal8Bit().constData(), O_RDONLY);
    if (srcFd < 0) {
        return false;
    }

    int destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFd < 0) {
        close(srcFd);
        return false;
    }

    struct stat srcStat;
    if (fstat(srcFd, &srcStat) < 0) {
        close(srcFd);
        close(destFd);
        return false;
    }

    qint64 totalSize = srcStat.st_size;
    qint64 copied = 0;

    while (copied < totalSize && !observer->shouldStop()) {
        // Check for pause state and wait efficiently
        if (observer->shouldPause()) {
            observer->waitWhilePaused();
        }

        if (observer->shouldStop()) {
            break;
        }

        ssize_t result = copy_file_range(srcFd, nullptr, destFd, nullptr,
                                         totalSize - copied, 0);

        if (result < 0) {
            // Check if we should fallback
            if (shouldFallback(errno)) {
                close(srcFd);
                close(destFd);
                return false;   // Will trigger fallback
            } else {
                // Permanent error
                close(srcFd);
                close(destFd);
                unlink(dest.toLocal8Bit().constData());
                return false;
            }
        }

        if (result == 0) {
            break;   // EOF
        }

        copied += result;
        m_copiedBytes += result;

        // Report progress
        if (observer) {
            observer->onProgressUpdate(m_copiedBytes, m_totalBytes);
        }
    }

    close(srcFd);
    close(destFd);

    return !observer->shouldStop() && copied == totalSize;
#else
    Q_UNUSED(source)
    Q_UNUSED(dest)
    Q_UNUSED(observer)
    return false;   // Will trigger fallback
#endif
}

bool SyncCopyAlgorithm::copyFileChunked(const QString &source, const QString &dest,
                                        ProgressObserver *observer)
{
    int srcFd = open(source.toLocal8Bit().constData(), O_RDONLY);
    if (srcFd < 0) {
        if (observer) {
            observer->onError(QString("Failed to open source file: %1 (errno: %2)")
                                      .arg(source)
                                      .arg(strerror(errno)));
        }
        return false;
    }

    qint64 totalSize = getFileSize(source);
    if (totalSize < 0) {
        close(srcFd);
        return false;
    }

    // Try to open with O_DIRECT first, fallback to normal mode if needed
    FileWriter writer = openDestinationFile(dest, WriteMode::Direct);
    if (writer.fd < 0) {
        close(srcFd);
        return false;
    }

    bool directModeSwitched = false;
    bool result = copyWithWriter(srcFd, writer, totalSize, dest, observer, directModeSwitched);

    close(srcFd);

    // dd-style: If we switched from O_DIRECT to normal mode during copy,
    // ensure data is synced to disk and drop cache
    if (result && directModeSwitched) {
        qDebug() << "Syncing data to disk (fsync) after O_DIRECT fallback for:" << dest;
        if (fsync(writer.fd) != 0) {
            qDebug() << "fsync failed:" << strerror(errno);
        }

        // dd also uses fadvise to drop cache after switching from O_DIRECT
        off_t currentPos = lseek(writer.fd, 0, SEEK_CUR);
        if (currentPos > 0) {
            if (posix_fadvise(writer.fd, currentPos, 0, POSIX_FADV_DONTNEED) != 0) {
                qDebug() << "posix_fadvise failed:" << strerror(errno);
            }
        }
    } else if (result && writer.mode == WriteMode::Normal) {
        // For normal mode files, ensure sync
        performSyncOperation(writer.fd, dest);
    }

    close(writer.fd);

    return result;
}

SyncCopyAlgorithm::FileWriter SyncCopyAlgorithm::openDestinationFile(const QString &dest, WriteMode preferredMode)
{
    int destFd = -1;
    WriteMode actualMode = preferredMode;

    if (preferredMode == WriteMode::Direct) {
        // Try O_DIRECT first
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0666);

        if (destFd < 0 && (errno == EINVAL || errno == ENOTSUP)) {
            // O_DIRECT not supported, fallback to normal mode
            qDebug() << "O_DIRECT not supported, falling back to normal mode for:" << dest;
            actualMode = WriteMode::Normal;
        }
    }

    if (destFd < 0) {
        // Open in normal mode (no special flags)
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        actualMode = WriteMode::Normal;
    }

    qDebug() << "Using" << (actualMode == WriteMode::Direct ? "O_DIRECT" : "normal")
             << "mode for file:" << dest;

    return FileWriter(destFd, actualMode);
}

SyncCopyAlgorithm::FileWriter SyncCopyAlgorithm::reopenDestinationFileForResume(const QString &dest, WriteMode preferredMode)
{
    int destFd = -1;
    WriteMode actualMode = preferredMode;

    if (preferredMode == WriteMode::Direct) {
        // Try O_DIRECT first - use O_WRONLY (no truncate) and seek to end
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_DIRECT, 0666);

        if (destFd < 0 && (errno == EINVAL || errno == ENOTSUP)) {
            // O_DIRECT not supported, fallback to normal mode
            qDebug() << "O_DIRECT not supported for resume, falling back to normal mode for:" << dest;
            actualMode = WriteMode::Normal;
        }
    }

    if (destFd < 0) {
        // Open in normal mode (no special flags, no truncate)
        destFd = open(dest.toLocal8Bit().constData(), O_WRONLY, 0666);
        actualMode = WriteMode::Normal;
    }

    if (destFd >= 0) {
        // Seek to end of file to continue writing where we left off
        if (lseek(destFd, 0, SEEK_END) < 0) {
            qDebug() << "Failed to seek to end of file for resume:" << strerror(errno);
            close(destFd);
            return FileWriter(-1, actualMode);
        }
    }

    qDebug() << "Reopened for resume using" << (actualMode == WriteMode::Direct ? "O_DIRECT" : "normal")
             << "mode for file:" << dest;

    return FileWriter(destFd, actualMode);
}

bool SyncCopyAlgorithm::copyWithWriter(int srcFd, FileWriter writer, qint64 totalSize,
                                       const QString &dest, ProgressObserver *observer, bool &directModeSwitched)
{
    // Dynamic chunk size based on file size, always align for potential O_DIRECT usage
    qint64 baseChunkSize = ChunkStrategy::getOptimalChunkSize(totalSize);
    qint64 chunkSize = ((baseChunkSize + writer.alignment - 1) / writer.alignment) * writer.alignment;

    qDebug() << "Using aligned chunk size:" << chunkSize << "bytes (base:" << baseChunkSize
             << ") for file of size:" << totalSize << "bytes";

    char *buffer = allocateAlignedBuffer(chunkSize, writer.alignment);
    if (!buffer) {
        if (observer) {
            observer->onError(QString("Failed to allocate aligned buffer: %1")
                                      .arg(strerror(errno)));
        }
        return false;
    }

    qint64 copied = 0;
    bool success = true;
    bool directModeActive = (writer.mode == WriteMode::Direct);
    directModeSwitched = false;

    while (copied < totalSize && (!observer || !observer->shouldStop())) {
        // Handle pause/resume
        if (observer && observer->shouldPause()) {
            FileWriter writerCopy = writer;   // Create a copy for modification
            if (!handlePauseResume(writerCopy, dest, observer)) {
                success = false;
                break;
            }
            // After resume, need to seek source file to correct position
            if (lseek(srcFd, copied, SEEK_SET) < 0) {
                qDebug() << "Failed to seek source file after resume:" << strerror(errno);
                success = false;
                break;
            }
            // Update writer with the reopened file descriptor
            writer = writerCopy;
        }

        if (observer && observer->shouldStop()) {
            success = false;
            break;
        }

        qint64 remaining = totalSize - copied;
        qint64 toRead = qMin(chunkSize, remaining);

        // For O_DIRECT, ensure read size is aligned (except for the very last read)
        if (writer.mode == WriteMode::Direct && toRead < chunkSize && toRead % writer.alignment != 0) {
            toRead = ((toRead + writer.alignment - 1) / writer.alignment) * writer.alignment;
            toRead = qMin(toRead, remaining);
        }

        ssize_t bytesRead = read(srcFd, buffer, toRead);
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            success = false;
            break;
        }

        if (bytesRead == 0) {
            break;   // EOF
        }

        // Only write the actual bytes we need
        qint64 actualBytesToWrite = qMin((qint64)bytesRead, totalSize - copied);

        ssize_t bytesWritten = 0;

        while (bytesWritten < actualBytesToWrite && (!observer || !observer->shouldStop())) {
            ssize_t written = write(writer.fd, buffer + bytesWritten,
                                    actualBytesToWrite - bytesWritten);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }

                // dd-style fallback: If O_DIRECT write fails due to alignment issues,
                // dynamically remove O_DIRECT flag and continue with regular I/O
                if (directModeActive && errno == EINVAL) {
                    qDebug() << "O_DIRECT write failed (alignment issue), removing O_DIRECT flag for:" << dest;

                    // Get current flags and remove O_DIRECT
                    int flags = fcntl(writer.fd, F_GETFL);
                    if (flags != -1) {
                        flags &= ~O_DIRECT;
                        if (fcntl(writer.fd, F_SETFL, flags) == 0) {
                            directModeActive = false;
                            directModeSwitched = true;
                            qDebug() << "Successfully removed O_DIRECT, continuing with regular I/O";
                            continue;   // Retry the write without O_DIRECT
                        }
                    }
                }

                perror("Copy error!!");
                success = false;
                break;
            }
            bytesWritten += written;
        }

        if (!success) {
            break;
        }

        copied += actualBytesToWrite;
        m_copiedBytes += actualBytesToWrite;

        // Report progress
        if (observer) {
            observer->onProgressUpdate(m_copiedBytes, m_totalBytes);
        }
    }

    free(buffer);

    // Clean up on failure
    if (!success) {
        unlink(dest.toLocal8Bit().constData());
    }

    return success && (!observer || !observer->shouldStop()) && copied == totalSize;
}

char *SyncCopyAlgorithm::allocateAlignedBuffer(size_t size, size_t alignment)
{
    char *buffer = nullptr;
    if (posix_memalign((void **)&buffer, alignment, size) != 0) {
        return nullptr;
    }
    return buffer;
}

bool SyncCopyAlgorithm::handlePauseResume(FileWriter &writer, const QString &dest, ProgressObserver *observer)
{
    // Sync data before pausing (Windows-style behavior)
    performSyncOperation(writer.fd, dest);
    close(writer.fd);

    observer->waitWhilePaused();

    // Reopen for resume (append mode, not truncate)
    FileWriter newWriter = reopenDestinationFileForResume(dest, writer.mode);
    if (newWriter.fd < 0) {
        return false;
    }

    writer = newWriter;
    return true;
}

bool SyncCopyAlgorithm::shouldFallback(int errorCode) const
{
    // These errors suggest copy_file_range is not supported or not suitable
    return errorCode == ENOSYS ||   // System call not implemented
            errorCode == EXDEV ||   // Cross-device copy
            errorCode == EINVAL ||   // Invalid arguments
            errorCode == EBADF;   // Bad file descriptor
}

bool SyncCopyAlgorithm::copyDirectoryRecursive(const QString &source, const QString &dest,
                                               ProgressObserver *observer)
{
    QDir sourceDir(source);
    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (isCancelled()) {
            return false;
        }

        waitIfPaused();

        QString sourcePath = entry.absoluteFilePath();
        QString destPath = dest + "/" + entry.fileName();

        if (entry.isFile()) {
            if (!copyFileInternal(sourcePath, destPath, observer)) {
                return false;
            }
        } else if (entry.isDir()) {
            if (!FileSystemUtils::ensureDirectoryExists(destPath)) {
                if (observer) {
                    observer->onError(QString("Failed to create directory: %1").arg(destPath));
                }
                return false;
            }

            if (!copyDirectoryRecursive(sourcePath, destPath, observer)) {
                return false;
            }
        }
    }

    return true;
}

bool SyncCopyAlgorithm::createDirectoryStructure(const QString &source, const QString &dest)
{
    return FileSystemUtils::ensureDirectoryExists(dest);
}

void SyncCopyAlgorithm::performSyncOperation(int fd, const QString &path) const
{
    qDebug() << "Performing sync operation for:" << path;
    if (fd >= 0) {
        syncfs(fd);
        qDebug() << "Performing sync operation end";
    }
}

qint64 SyncCopyAlgorithm::getFileSize(const QString &filePath) const
{
    struct stat fileStat;
    if (stat(filePath.toLocal8Bit().constData(), &fileStat) == 0) {
        return fileStat.st_size;
    }
    return -1;
}

void SyncCopyAlgorithm::waitIfPaused()
{
    // Pause handling is now done by CopyWorker
    // This method is no longer needed
}

bool SyncCopyAlgorithm::isCancelled() const
{
    // Cancel handling is now done by CopyWorker
    return false;
}
