#include "DefaultCopyAlgorithm.h"
#include "core/FileSystemUtils.h"
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <fcntl.h>
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

DefaultCopyAlgorithm::DefaultCopyAlgorithm(QObject *parent)
    : CopyAlgorithm(parent), m_totalBytes(0), m_copiedBytes(0), m_isDirectoryCopy(false)
{
}

bool DefaultCopyAlgorithm::copyFile(const QString &source, const QString &dest,
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

    return copyFileInternal(source, dest, observer);
}

bool DefaultCopyAlgorithm::copyFileInternal(const QString &source, const QString &dest,
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
            observer->onError(QString("Failed to copy file: %1 to %2").arg(source, dest));
        }
    }

    return success;
}

bool DefaultCopyAlgorithm::copyDirectory(const QString &source, const QString &dest,
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

    // Reset directory copy mode
    m_isDirectoryCopy = false;

    return result;
}

qint64 DefaultCopyAlgorithm::calculateTotalSize(const QString &path)
{
    return FileSystemUtils::calculateTotalSize(path);
}

bool DefaultCopyAlgorithm::supportsPause() const
{
    return true;
}

QString DefaultCopyAlgorithm::getName() const
{
    return "Default Algorithm (copy_file_range + chunked fallback)";
}

void DefaultCopyAlgorithm::pause()
{
    // Pause/resume is now handled by CopyWorker
    emit paused();
}

void DefaultCopyAlgorithm::resume()
{
    // Pause/resume is now handled by CopyWorker
    emit resumed();
}

void DefaultCopyAlgorithm::cancel()
{
    // Cancel is now handled by CopyWorker
    emit cancelled();
}

bool DefaultCopyAlgorithm::copyFileRange(const QString &source, const QString &dest,
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

bool DefaultCopyAlgorithm::copyFileChunked(const QString &source, const QString &dest,
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

    int destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFd < 0) {
        if (observer) {
            observer->onError(QString("Failed to open destination file: %1 (errno: %2)")
                                      .arg(dest)
                                      .arg(strerror(errno)));
        }
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
    char buffer[CHUNK_SIZE];

    while (copied < totalSize && !observer->shouldStop()) {
        // Check for pause state and wait efficiently
        if (observer->shouldPause()) {
            // syncfs(destFd);
            close(destFd);
            observer->waitWhilePaused();
            destFd = open(dest.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (observer->shouldStop()) {
            break;
        }

        qint64 toRead = std::min(CHUNK_SIZE, totalSize - copied);
        ssize_t bytesRead = read(srcFd, buffer, toRead);

        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;   // Interrupted, try again
            }
            close(srcFd);
            close(destFd);
            unlink(dest.toLocal8Bit().constData());
            return false;
        }

        if (bytesRead == 0) {
            break;   // EOF
        }

        ssize_t bytesWritten = 0;
        while (bytesWritten < bytesRead && !observer->shouldStop()) {
            ssize_t written = write(destFd, buffer + bytesWritten,
                                    bytesRead - bytesWritten);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                close(srcFd);
                close(destFd);
                unlink(dest.toLocal8Bit().constData());
                return false;
            }
            bytesWritten += written;
        }

        copied += bytesRead;
        m_copiedBytes += bytesRead;

        // Report progress
        if (observer) {
            observer->onProgressUpdate(m_copiedBytes, m_totalBytes);
        }
    }

    close(srcFd);
    close(destFd);

    return !observer->shouldStop() && copied == totalSize;
}

bool DefaultCopyAlgorithm::shouldFallback(int errorCode) const
{
    // These errors suggest copy_file_range is not supported or not suitable
    return errorCode == ENOSYS ||   // System call not implemented
            errorCode == EXDEV ||   // Cross-device copy
            errorCode == EINVAL ||   // Invalid arguments
            errorCode == EBADF;   // Bad file descriptor
}

bool DefaultCopyAlgorithm::copyDirectoryRecursive(const QString &source, const QString &dest,
                                                  ProgressObserver *observer)
{
    QDir sourceDir(source);
    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

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

bool DefaultCopyAlgorithm::createDirectoryStructure(const QString &source, const QString &dest)
{
    return FileSystemUtils::ensureDirectoryExists(dest);
}

void DefaultCopyAlgorithm::waitIfPaused()
{
    // Pause handling is now done by CopyWorker
    // This method is no longer needed
}

bool DefaultCopyAlgorithm::isCancelled() const
{
    // Cancel handling is now done by CopyWorker
    return false;
}
