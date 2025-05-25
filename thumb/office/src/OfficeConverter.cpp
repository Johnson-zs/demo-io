#include "OfficeConverter.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>
#include <QCoreApplication>

OfficeConverter::OfficeConverter(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_progressTimer(new QTimer(this))
    , m_progressValue(0)
{
    m_progressTimer->setInterval(100); // 100ms更新一次进度
    connect(m_progressTimer, &QTimer::timeout, this, &OfficeConverter::onProgressTimer);
}

OfficeConverter::~OfficeConverter()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

void OfficeConverter::convertToPdf(const QString &inputFile, const QString &outputFile)
{
    if (!QFile::exists(inputFile)) {
        emit conversionError(QString("输入文件不存在: %1").arg(inputFile));
        return;
    }
    
    QFileInfo inputInfo(inputFile);
    if (inputInfo.size() == 0) {
        emit conversionError(QString("输入文件为空: %1").arg(inputFile));
        return;
    }
    
    if (!isLibreOfficeAvailable()) {
        emit conversionError("LibreOffice 未安装或不可用。请安装 LibreOffice 以支持文件转换。");
        return;
    }
    
    qDebug() << "开始转换文件:" << inputFile << "大小:" << inputInfo.size() << "字节";
    qDebug() << "输出文件:" << outputFile;
    
    // 检查缓存
    QString cachedPdf = getCachedPdfPath(inputFile);
    QFileInfo cachedInfo(cachedPdf);
    
    if (cachedInfo.exists() && cachedInfo.size() > 0 && cachedInfo.lastModified() >= inputInfo.lastModified()) {
        qDebug() << "使用缓存的PDF:" << cachedPdf << "大小:" << cachedInfo.size() << "字节";
        emit conversionProgress(100);
        emit conversionFinished(cachedPdf);
        return;
    }
    
    m_outputFile = outputFile;
    
    // 创建输出目录
    QString outputDir = QFileInfo(outputFile).absolutePath();
    if (!QDir().mkpath(outputDir)) {
        emit conversionError(QString("无法创建输出目录: %1").arg(outputDir));
        return;
    }
    qDebug() << "输出目录:" << outputDir;
    
    // 如果进程正在运行，先终止
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
    
    // 创建新进程
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &OfficeConverter::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &OfficeConverter::onProcessError);
    connect(m_process, &QProcess::started, this, &OfficeConverter::onProcessStarted);
    
    // 构建LibreOffice命令
    QString libreOfficePath = findLibreOfficePath();
    QStringList arguments;
    
    arguments << "--headless"
              << "--convert-to" << "pdf"
              << "--outdir" << outputDir
              << inputFile;
    
    qDebug() << "开始转换:" << libreOfficePath << arguments;
    
    m_progressValue = 0;
    emit conversionProgress(0);
    
    // 启动转换进程
    m_process->start(libreOfficePath, arguments);
    
    if (!m_process->waitForStarted(5000)) {
        emit conversionError("LibreOffice启动超时");
        return;
    }
}

bool OfficeConverter::isLibreOfficeAvailable()
{
    return !findLibreOfficePath().isEmpty();
}

QString OfficeConverter::getCacheDir()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cacheDir += "/office_preview";
    QDir().mkpath(cacheDir);
    return cacheDir;
}

QString OfficeConverter::getCachedPdfPath(const QString &inputFile)
{
    QFileInfo fileInfo(inputFile);
    
    // 使用文件路径和修改时间生成唯一的缓存文件名
    QString data = inputFile + QString::number(fileInfo.lastModified().toSecsSinceEpoch());
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    QString fileName = hash.toHex() + ".pdf";
    
    return getCacheDir() + "/" + fileName;
}

QStringList OfficeConverter::getPossibleLibreOfficePaths()
{
    QStringList paths;
    
#ifdef Q_OS_WIN
    // Windows路径
    paths << "C:/Program Files/LibreOffice/program/soffice.exe"
          << "C:/Program Files (x86)/LibreOffice/program/soffice.exe"
          << "soffice.exe";
#elif defined(Q_OS_MAC)
    // macOS路径
    paths << "/Applications/LibreOffice.app/Contents/MacOS/soffice"
          << "soffice";
#else
    // Linux路径
    paths << "/usr/bin/libreoffice"
          << "/usr/local/bin/libreoffice"
          << "/opt/libreoffice/program/soffice"
          << "/snap/bin/libreoffice"
          << "libreoffice"
          << "soffice";
#endif
    
    return paths;
}

QString OfficeConverter::findLibreOfficePath()
{
    static QString cachedPath;
    if (!cachedPath.isEmpty()) {
        return cachedPath;
    }
    
    QStringList paths = getPossibleLibreOfficePaths();
    
    for (const QString &path : paths) {
        QProcess testProcess;
        testProcess.start(path, QStringList() << "--version");
        if (testProcess.waitForFinished(3000) && testProcess.exitCode() == 0) {
            cachedPath = path;
            qDebug() << "找到LibreOffice:" << path;
            return cachedPath;
        }
    }
    
    qWarning() << "未找到可用的LibreOffice安装";
    return QString();
}

void OfficeConverter::onProcessStarted()
{
    qDebug() << "LibreOffice进程已启动";
    m_progressTimer->start();
}

void OfficeConverter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_progressTimer->stop();
    
    // 读取所有输出信息用于调试
    QString standardOutput = m_process->readAllStandardOutput();
    QString standardError = m_process->readAllStandardError();
    
    qDebug() << "LibreOffice退出码:" << exitCode;
    qDebug() << "标准输出:" << standardOutput;
    qDebug() << "错误输出:" << standardError;
    
    if (exitStatus == QProcess::CrashExit) {
        emit conversionError("LibreOffice进程崩溃");
        return;
    }
    
    if (exitCode != 0) {
        emit conversionError(QString("转换失败 (退出码: %1): %2").arg(exitCode).arg(standardError));
        return;
    }
    
    // LibreOffice使用输入文件的基本名称来生成输出文件，而不是我们指定的输出文件名
    QFileInfo outputInfo(m_outputFile);
    QString outputDir = outputInfo.absolutePath();
    
    // 获取输入文件名（从命令参数中）
    QStringList args = m_process->arguments();
    QString inputFile = args.last(); // 最后一个参数是输入文件
    QFileInfo inputInfo(inputFile);
    QString expectedFileName = inputInfo.baseName() + ".pdf";
    QString actualOutputFile = outputDir + "/" + expectedFileName;
    
    qDebug() << "期望的输出文件:" << m_outputFile;
    qDebug() << "LibreOffice实际生成的文件:" << actualOutputFile;
    
    // 检查实际生成的文件是否存在且有内容
    QFileInfo actualFileInfo(actualOutputFile);
    if (actualFileInfo.exists() && actualFileInfo.size() > 0) {
        // 如果实际文件和期望文件不同，进行重命名
        if (actualOutputFile != m_outputFile) {
            // 删除目标文件（如果存在）
            if (QFile::exists(m_outputFile)) {
                QFile::remove(m_outputFile);
            }
            // 重命名文件
            if (!QFile::rename(actualOutputFile, m_outputFile)) {
                qWarning() << "无法重命名文件从" << actualOutputFile << "到" << m_outputFile;
                // 如果重命名失败，使用实际文件路径
                m_outputFile = actualOutputFile;
            }
        }
        
        // 再次检查最终文件
        QFileInfo finalFileInfo(m_outputFile);
        if (finalFileInfo.exists() && finalFileInfo.size() > 0) {
            // 复制到缓存
            QString cachedPath = getCachedPdfPath(inputFile);
            
            // 确保缓存目录存在
            QDir().mkpath(QFileInfo(cachedPath).absolutePath());
            
            // 删除旧的缓存文件（如果存在）
            if (QFile::exists(cachedPath)) {
                QFile::remove(cachedPath);
            }
            
            if (QFile::copy(m_outputFile, cachedPath)) {
                qDebug() << "缓存文件已创建:" << cachedPath;
                QFileInfo cachedInfo(cachedPath);
                qDebug() << "缓存文件大小:" << cachedInfo.size() << "字节";
            } else {
                qWarning() << "无法创建缓存文件:" << cachedPath;
            }
            
            emit conversionProgress(100);
            emit conversionFinished(m_outputFile);
            qDebug() << "转换完成:" << m_outputFile << "大小:" << finalFileInfo.size() << "字节";
        } else {
            emit conversionError(QString("最终文件检查失败: %1").arg(m_outputFile));
        }
    } else {
        // 尝试查找其他可能的文件
        QDir outputDirObj(outputDir);
        QStringList filters;
        filters << "*.pdf";
        QStringList pdfFiles = outputDirObj.entryList(filters, QDir::Files);
        
        QString errorMsg = QString("转换失败，找不到有效的输出文件。\n期望文件: %1\n实际文件: %2")
                          .arg(m_outputFile).arg(actualOutputFile);
        
        if (!pdfFiles.isEmpty()) {
            errorMsg += "\n输出目录中的PDF文件:";
            for (const QString &file : pdfFiles) {
                QString fullPath = outputDir + "/" + file;
                QFileInfo info(fullPath);
                errorMsg += QString("\n  %1 (大小: %2 字节)").arg(file).arg(info.size());
            }
        }
        
        // 添加LibreOffice的输出信息到错误消息
        if (!standardError.isEmpty()) {
            errorMsg += "\nLibreOffice错误信息: " + standardError;
        }
        if (!standardOutput.isEmpty()) {
            errorMsg += "\nLibreOffice输出信息: " + standardOutput;
        }
        
        emit conversionError(errorMsg);
    }
}

void OfficeConverter::onProcessError(QProcess::ProcessError error)
{
    m_progressTimer->stop();
    
    QString errorString;
    switch (error) {
    case QProcess::FailedToStart:
        errorString = "LibreOffice启动失败。请检查是否已正确安装LibreOffice。";
        break;
    case QProcess::Crashed:
        errorString = "LibreOffice进程崩溃";
        break;
    case QProcess::Timedout:
        errorString = "LibreOffice进程超时";
        break;
    default:
        errorString = "LibreOffice进程出现未知错误";
        break;
    }
    
    emit conversionError(errorString);
}

void OfficeConverter::onProgressTimer()
{
    // 模拟进度更新
    if (m_progressValue < 90) {
        m_progressValue += 5;
        emit conversionProgress(m_progressValue);
    }
} 