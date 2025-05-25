#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTemporaryDir>
#include <QTimer>
#include <QFileSystemWatcher>

class OfficeConverter : public QObject
{
    Q_OBJECT

public:
    explicit OfficeConverter(QObject *parent = nullptr);
    ~OfficeConverter();

    // 转换Office文件为PDF
    void convertToPdf(const QString &inputFile, const QString &outputFile);
    
    // 检查LibreOffice是否可用
    static bool isLibreOfficeAvailable();
    
    // 获取转换缓存目录
    static QString getCacheDir();
    
    // 根据输入文件生成缓存PDF路径
    static QString getCachedPdfPath(const QString &inputFile);

signals:
    void conversionProgress(int percentage);
    void conversionFinished(const QString &pdfPath);
    void conversionError(const QString &error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessStarted();
    void onProgressTimer();

private:
    QProcess *m_process;
    QTimer *m_progressTimer;
    QString m_outputFile;
    int m_progressValue;
    
    // LibreOffice路径检测
    static QStringList getPossibleLibreOfficePaths();
    static QString findLibreOfficePath();
}; 