#include "thumbnailgenerator.h"
#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>

ThumbnailGenerator::ThumbnailGenerator(QObject *parent)
    : QObject(parent)
    , m_libreOfficeProcess(nullptr)
    , m_checkTimer(new QTimer(this))
    , m_isGenerating(false)
    , m_expectedPages(0)
{
    connect(m_checkTimer, &QTimer::timeout, this, &ThumbnailGenerator::checkThumbnailFiles);
}

ThumbnailGenerator::~ThumbnailGenerator()
{
    stopGeneration();
    cleanupTempDirectory();
}

void ThumbnailGenerator::generateThumbnails(const QString &pptFilePath)
{
    if (m_isGenerating) {
        qWarning() << "Already generating thumbnails";
        return;
    }
    
    if (!checkLibreOfficeAvailable()) {
        emit generationError("LibreOffice未安装或不可用。请安装LibreOffice。");
        return;
    }
    
    m_currentPptFile = pptFilePath;
    m_isGenerating = true;
    m_generatedFiles.clear();
    m_expectedPages = 0;
    
    setupTempDirectory();
    
    // 创建LibreOffice进程
    if (m_libreOfficeProcess) {
        m_libreOfficeProcess->kill();
        m_libreOfficeProcess->deleteLater();
    }
    
    m_libreOfficeProcess = new QProcess(this);
    connect(m_libreOfficeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ThumbnailGenerator::onConversionFinished);
    connect(m_libreOfficeProcess, &QProcess::errorOccurred,
            this, &ThumbnailGenerator::onConversionError);
    
    // 构建LibreOffice命令行参数
    QString libreOfficePath = getLibreOfficePath();
    QStringList arguments;
    arguments << "--headless"
              << "--convert-to" << "png"
              << "--outdir" << m_tempDir
              << m_currentPptFile;
    
    qDebug() << "Starting LibreOffice with arguments:" << arguments;
    
    // 启动转换进程
    m_libreOfficeProcess->start(libreOfficePath, arguments);
    
    if (!m_libreOfficeProcess->waitForStarted(5000)) {
        emit generationError("无法启动LibreOffice进程");
        m_isGenerating = false;
        return;
    }
    
    // 启动检查定时器
    m_checkTimer->start(CHECK_INTERVAL_MS);
}

void ThumbnailGenerator::stopGeneration()
{
    if (!m_isGenerating) {
        return;
    }
    
    m_isGenerating = false;
    m_checkTimer->stop();
    
    if (m_libreOfficeProcess && m_libreOfficeProcess->state() != QProcess::NotRunning) {
        m_libreOfficeProcess->kill();
        m_libreOfficeProcess->waitForFinished(3000);
    }
}

void ThumbnailGenerator::setupTempDirectory()
{
    cleanupTempDirectory();
    
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_tempDir = tempPath + "/ppt_thumbnails_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QDir dir;
    if (!dir.mkpath(m_tempDir)) {
        qWarning() << "Failed to create temp directory:" << m_tempDir;
        m_tempDir = tempPath; // 使用默认临时目录
    }
    
    qDebug() << "Using temp directory:" << m_tempDir;
}

void ThumbnailGenerator::cleanupTempDirectory()
{
    if (!m_tempDir.isEmpty() && QDir(m_tempDir).exists()) {
        QDir tempDir(m_tempDir);
        
        // 删除目录中的所有文件
        QStringList files = tempDir.entryList(QDir::Files);
        for (const QString &file : files) {
            tempDir.remove(file);
        }
        
        // 删除目录
        tempDir.rmdir(".");
        qDebug() << "Cleaned up temp directory:" << m_tempDir;
    }
}

bool ThumbnailGenerator::checkLibreOfficeAvailable()
{
    QString libreOfficePath = getLibreOfficePath();
    
    QProcess testProcess;
    testProcess.start(libreOfficePath, QStringList() << "--version");
    
    if (!testProcess.waitForStarted(3000)) {
        return false;
    }
    
    testProcess.waitForFinished(5000);
    return testProcess.exitCode() == 0;
}

QString ThumbnailGenerator::getLibreOfficePath()
{
    // 常见的LibreOffice路径
    QStringList possiblePaths = {
        "/usr/bin/libreoffice",
        "/usr/local/bin/libreoffice",
        "/opt/libreoffice/program/soffice",
        "/Applications/LibreOffice.app/Contents/MacOS/soffice",
        "libreoffice",  // 系统PATH中
        "soffice"       // 备选名称
    };
    
    for (const QString &path : possiblePaths) {
        QProcess testProcess;
        testProcess.start(path, QStringList() << "--version");
        
        if (testProcess.waitForStarted(1000)) {
            testProcess.waitForFinished(3000);
            if (testProcess.exitCode() == 0) {
                qDebug() << "Found LibreOffice at:" << path;
                return path;
            }
        }
    }
    
    qWarning() << "LibreOffice not found in common paths";
    return "libreoffice"; // 默认尝试系统PATH
}

void ThumbnailGenerator::onConversionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "LibreOffice process finished with exit code:" << exitCode 
             << "status:" << exitStatus;
    
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString errorOutput = m_libreOfficeProcess->readAllStandardError();
        emit generationError(QString("LibreOffice转换失败 (退出代码: %1)\n%2")
                           .arg(exitCode).arg(errorOutput));
        m_isGenerating = false;
        m_checkTimer->stop();
        return;
    }
    
    // 继续检查文件，LibreOffice可能需要一些时间来完成写入
    qDebug() << "LibreOffice finished successfully, checking for generated files...";
}

void ThumbnailGenerator::onConversionError(QProcess::ProcessError error)
{
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = "LibreOffice进程启动失败";
            break;
        case QProcess::Crashed:
            errorString = "LibreOffice进程崩溃";
            break;
        case QProcess::Timedout:
            errorString = "LibreOffice进程超时";
            break;
        case QProcess::ReadError:
        case QProcess::WriteError:
            errorString = "LibreOffice进程读写错误";
            break;
        default:
            errorString = "LibreOffice进程未知错误";
            break;
    }
    
    emit generationError(errorString);
    m_isGenerating = false;
    m_checkTimer->stop();
}

void ThumbnailGenerator::checkThumbnailFiles()
{
    if (!m_isGenerating) {
        m_checkTimer->stop();
        return;
    }
    
    QDir tempDir(m_tempDir);
    QStringList filters;
    filters << "*.png";
    
    QStringList currentFiles = tempDir.entryList(filters, QDir::Files, QDir::Name);
    
    // 检查是否有新文件生成
    for (const QString &file : currentFiles) {
        if (!m_generatedFiles.contains(file)) {
            m_generatedFiles.append(file);
            
            QString fullPath = tempDir.absoluteFilePath(file);
            
            // 从文件名提取页码
            // LibreOffice通常生成如 "presentation-01.png" 的文件名
            QRegularExpression pageRegex(R"((\d+)\.png$)");
            QRegularExpressionMatch match = pageRegex.match(file);
            
            int pageNumber = 1;
            if (match.hasMatch()) {
                pageNumber = match.captured(1).toInt();
            } else {
                // 如果没有页码，按照出现顺序分配
                pageNumber = m_generatedFiles.size();
            }
            
            emit thumbnailGenerated(fullPath, pageNumber);
            qDebug() << "Generated thumbnail for page" << pageNumber << ":" << fullPath;
        }
    }
    
    // 检查是否完成
    // 如果LibreOffice进程已结束且有文件生成，认为完成
    if (m_libreOfficeProcess && 
        m_libreOfficeProcess->state() == QProcess::NotRunning && 
        !currentFiles.isEmpty()) {
        
        qDebug() << "Generation completed. Total files:" << currentFiles.size();
        
        m_isGenerating = false;
        m_checkTimer->stop();
        
        emit generationFinished();
    }
    
    // 超时检查
    static int checkCount = 0;
    checkCount++;
    
    if (checkCount * CHECK_INTERVAL_MS > MAX_WAIT_TIME_MS) {
        qWarning() << "Timeout waiting for thumbnail generation";
        emit generationError("生成缩略图超时");
        m_isGenerating = false;
        m_checkTimer->stop();
        checkCount = 0;
    }
} 