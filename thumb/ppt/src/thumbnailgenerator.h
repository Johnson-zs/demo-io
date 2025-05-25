#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QObject>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>

class ThumbnailGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ThumbnailGenerator(QObject *parent = nullptr);
    ~ThumbnailGenerator();

    void generateThumbnails(const QString &pptFilePath);
    void stopGeneration();
    bool isGenerating() const { return m_isGenerating; }

signals:
    void thumbnailGenerated(const QString &imagePath, int pageNumber);
    void generationFinished();
    void generationError(const QString &error);
    void progressUpdated(int current, int total);

private slots:
    void onConversionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConversionError(QProcess::ProcessError error);
    void checkThumbnailFiles();

private:
    void setupTempDirectory();
    void cleanupTempDirectory();
    bool checkLibreOfficeAvailable();
    QString getLibreOfficePath();

    QProcess *m_libreOfficeProcess;
    QString m_tempDir;
    QString m_currentPptFile;
    QTimer *m_checkTimer;
    
    bool m_isGenerating;
    int m_expectedPages;
    QStringList m_generatedFiles;
    
    static constexpr int CHECK_INTERVAL_MS = 500;
    static constexpr int MAX_WAIT_TIME_MS = 30000;  // 30秒超时
};

#endif // THUMBNAILGENERATOR_H 