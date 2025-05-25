#include "thumbnailextractor.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QBuffer>
#include <QImageReader>
#include <QStandardPaths>
#include <QCoreApplication>

// 使用第三方库或系统调用处理ZIP
#include <QProcess>
#include <QTemporaryDir>

ThumbnailExtractor::ThumbnailExtractor(QObject *parent)
    : QObject(parent)
{
}

ThumbnailExtractor::~ThumbnailExtractor() = default;

bool ThumbnailExtractor::extractThumbnails(const QString &pptFilePath)
{
    m_thumbnails.clear();
    
    QFileInfo fileInfo(pptFilePath);
    if (!fileInfo.exists()) {
        emit extractionError("文件不存在");
        return false;
    }
    
    QString suffix = fileInfo.suffix().toLower();
    
    qDebug() << "Extracting thumbnails from:" << pptFilePath << "Type:" << suffix;
    
    bool success = false;
    
    // 根据文件类型选择提取方法
    if (suffix == "pptx" || suffix == "docx" || suffix == "xlsx") {
        success = extractFromOfficeOpenXML(pptFilePath);
    } else if (suffix == "odp" || suffix == "odt" || suffix == "ods") {
        success = extractFromOpenDocument(pptFilePath);
    } else if (suffix == "ppt" || suffix == "doc" || suffix == "xls") {
        success = extractFromPowerPoint(pptFilePath);
    } else {
        emit extractionError("不支持的文件格式: " + suffix);
        return false;
    }
    
    if (success && !m_thumbnails.isEmpty()) {
        emit extractionFinished(m_thumbnails.size());
        return true;
    }
    
    return false;
}

bool ThumbnailExtractor::extractFromOfficeOpenXML(const QString &filePath)
{
    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        emit extractionError("无法创建临时目录");
        return false;
    }
    
    // 使用unzip命令解压文件
    QProcess unzipProcess;
    QStringList arguments;
    arguments << "-q" << "-o" << filePath << "-d" << tempDir.path();
    
    unzipProcess.start("unzip", arguments);
    if (!unzipProcess.waitForStarted(3000)) {
        emit extractionError("unzip命令不可用，无法提取ZIP文件");
        return false;
    }
    
    if (!unzipProcess.waitForFinished(10000)) {
        emit extractionError("解压超时");
        return false;
    }
    
    if (unzipProcess.exitCode() != 0) {
        emit extractionError("解压失败：" + unzipProcess.readAllStandardError());
        return false;
    }
    
    // 查找关系文件
    QString relsPath = tempDir.path() + "/_rels/.rels";
    QFile relsFile(relsPath);
    
    QString thumbnailPath;
    
    if (relsFile.exists() && relsFile.open(QIODevice::ReadOnly)) {
        QByteArray relsData = relsFile.readAll();
        relsFile.close();
        thumbnailPath = findThumbnailPathInRels(relsData);
    }
    
    // 尝试直接查找缩略图
    if (!thumbnailPath.isEmpty()) {
        QString fullThumbnailPath = tempDir.path() + "/" + thumbnailPath;
        QImage image(fullThumbnailPath);
        if (!image.isNull()) {
            m_thumbnails.append(image);
            emit thumbnailExtracted(image, 1);
            qDebug() << "Found thumbnail via relationships at:" << thumbnailPath;
            return true;
        }
    }
    
    // 尝试常见路径
    QStringList commonPaths = {
        "docProps/thumbnail.jpeg",
        "docProps/thumbnail.png", 
        "docProps/thumbnail.jpg"
    };
    
    for (const QString &path : commonPaths) {
        QString fullPath = tempDir.path() + "/" + path;
        QImage image(fullPath);
        if (!image.isNull()) {
            m_thumbnails.append(image);
            emit thumbnailExtracted(image, 1);
            qDebug() << "Found thumbnail at:" << path;
            return true;
        }
    }
    
    // 对于PPTX，尝试查找媒体图像
    if (filePath.endsWith(".pptx", Qt::CaseInsensitive)) {
        QDir mediaDir(tempDir.path() + "/ppt/media");
        if (mediaDir.exists()) {
            QStringList imageFiles = mediaDir.entryList(
                QStringList() << "*.png" << "*.jpg" << "*.jpeg",
                QDir::Files, QDir::Name
            );
            
            imageFiles.sort();
            
            for (int i = 0; i < imageFiles.size(); ++i) {
                QString imagePath = mediaDir.absoluteFilePath(imageFiles[i]);
                QImage image(imagePath);
                if (!image.isNull()) {
                    m_thumbnails.append(image);
                    emit thumbnailExtracted(image, i + 1);
                    qDebug() << "Found slide image:" << imageFiles[i];
                }
            }
            
            return !m_thumbnails.isEmpty();
        }
    }
    
    return false;
}

bool ThumbnailExtractor::extractFromOpenDocument(const QString &filePath)
{
    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        emit extractionError("无法创建临时目录");
        return false;
    }
    
    // 使用unzip命令解压文件
    QProcess unzipProcess;
    QStringList arguments;
    arguments << "-q" << "-o" << filePath << "-d" << tempDir.path();
    
    unzipProcess.start("unzip", arguments);
    if (!unzipProcess.waitForStarted(3000)) {
        emit extractionError("unzip命令不可用，无法提取ZIP文件");
        return false;
    }
    
    if (!unzipProcess.waitForFinished(10000)) {
        emit extractionError("解压超时");
        return false;
    }
    
    if (unzipProcess.exitCode() != 0) {
        emit extractionError("解压失败：" + unzipProcess.readAllStandardError());
        return false;
    }
    
    // OpenDocument标准缩略图路径
    QStringList thumbnailPaths = {
        "Thumbnails/thumbnail.png",
        "Thumbnails/thumbnail.jpg",
        "Thumbnails/thumbnail.jpeg"
    };
    
    for (const QString &path : thumbnailPaths) {
        QString fullPath = tempDir.path() + "/" + path;
        QImage image(fullPath);
        if (!image.isNull()) {
            m_thumbnails.append(image);
            emit thumbnailExtracted(image, 1);
            qDebug() << "Found OpenDocument thumbnail at:" << path;
            return true;
        }
    }
    
    return false;
}

bool ThumbnailExtractor::extractFromPowerPoint(const QString &filePath)
{
    // 对于老版本的PPT文件，我们暂时无法直接提取
    // 这些是二进制格式，需要专门的解析器
    Q_UNUSED(filePath)
    emit extractionError("老版本PPT格式暂不支持直接提取，请使用LibreOffice转换方式");
    return false;
}

QString ThumbnailExtractor::findThumbnailPathInRels(const QByteArray &relsData)
{
    QXmlStreamReader xml(relsData);
    
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("Relationship")) {
            const auto attributes = xml.attributes();
            QString type = attributes.value(QStringLiteral("Type")).toString();
            
            // 查找缩略图关系
            if (type == QLatin1String("http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail")) {
                QString target = attributes.value(QStringLiteral("Target")).toString();
                qDebug() << "Found thumbnail relationship target:" << target;
                return target;
            }
        }
    }
    
    if (xml.hasError()) {
        qWarning() << "XML parsing error:" << xml.errorString();
    }
    
    return QString();
} 