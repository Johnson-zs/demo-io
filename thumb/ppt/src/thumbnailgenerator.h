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

class ThumbnailExtractor;

class ThumbnailGenerator : public QObject
{
    Q_OBJECT

public:
    enum GenerationMethod {
        KDEExtraction,          // KDE风格：直接从文件提取
        LibreOfficeConversion,  // LibreOffice转换
        AutoDetect              // 自动检测：优先KDE方式，失败则用LibreOffice
    };

    explicit ThumbnailGenerator(QObject *parent = nullptr);
    ~ThumbnailGenerator();

    void generateThumbnails(const QString &pptFilePath);
    void stopGeneration();
    bool isGenerating() const { return m_isGenerating; }
    
    // 设置和获取生成方式
    void setGenerationMethod(GenerationMethod method) { m_generationMethod = method; }
    GenerationMethod getGenerationMethod() const { return m_generationMethod; }

signals:
    void thumbnailGenerated(const QString &imagePath, int pageNumber);
    void generationFinished();
    void generationError(const QString &error);
    void progressUpdated(int current, int total);
    void methodUsed(const QString &methodName);  // 通知使用了哪种方法

private slots:
    void onConversionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConversionError(QProcess::ProcessError error);
    void checkThumbnailFiles();
    void onExtractorFinished(int totalCount);
    void onExtractorError(const QString &error);
    void onThumbnailExtracted(const QImage &image, int pageNumber);

private:
    void setupTempDirectory();
    void cleanupTempDirectory();
    bool checkLibreOfficeAvailable();
    QString getLibreOfficePath();
    
    bool tryKDEExtraction(const QString &pptFilePath);
    void useLibreOfficeConversion(const QString &pptFilePath);
    QString saveImageToTemp(const QImage &image, int pageNumber);

    QProcess *m_libreOfficeProcess;
    ThumbnailExtractor *m_extractor;
    QString m_tempDir;
    QString m_currentPptFile;
    QTimer *m_checkTimer;
    
    bool m_isGenerating;
    int m_expectedPages;
    QStringList m_generatedFiles;
    GenerationMethod m_generationMethod;
    
    static constexpr int CHECK_INTERVAL_MS = 500;
    static constexpr int MAX_WAIT_TIME_MS = 30000;  // 30秒超时
};

#endif // THUMBNAILGENERATOR_H 