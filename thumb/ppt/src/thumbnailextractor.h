#ifndef THUMBNAILEXTRACTOR_H
#define THUMBNAILEXTRACTOR_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QStringList>

class ThumbnailExtractor : public QObject
{
    Q_OBJECT

public:
    explicit ThumbnailExtractor(QObject *parent = nullptr);
    ~ThumbnailExtractor();

    // 从PPT文件中直接提取缩略图
    bool extractThumbnails(const QString &pptFilePath);
    
    // 获取提取到的缩略图
    QList<QImage> getThumbnails() const { return m_thumbnails; }
    int getThumbnailCount() const { return m_thumbnails.size(); }

signals:
    void thumbnailExtracted(const QImage &image, int pageNumber);
    void extractionFinished(int totalCount);
    void extractionError(const QString &error);

private:
    bool extractFromOpenDocument(const QString &filePath);
    bool extractFromOfficeOpenXML(const QString &filePath);
    bool extractFromPowerPoint(const QString &filePath);
    
    QString findThumbnailPathInRels(const QByteArray &relsData);
    QImage loadImageFromZipEntry(class KZip *zip, const QString &entryPath);

    QList<QImage> m_thumbnails;
};

#endif // THUMBNAILEXTRACTOR_H 