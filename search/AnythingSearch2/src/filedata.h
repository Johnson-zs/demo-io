#ifndef FILEDATA_H
#define FILEDATA_H

#include <QString>
#include <QDateTime>
#include <QFileInfo>

struct FileData {
    QString name;
    QString path;
    QString type;
    qint64 size;
    QDateTime modified;
    bool isDirectory;
    
    static FileData fromFileInfo(const QFileInfo &fileInfo) {
        FileData data;
        data.name = fileInfo.fileName();
        data.path = fileInfo.absoluteFilePath();
        data.type = fileInfo.suffix().isEmpty() ? (fileInfo.isDir() ? "文件夹" : "文件") : fileInfo.suffix();
        data.size = fileInfo.size();
        data.modified = fileInfo.lastModified();
        data.isDirectory = fileInfo.isDir();
        return data;
    }
};

#endif // FILEDATA_H 