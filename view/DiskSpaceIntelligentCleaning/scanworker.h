#ifndef SCANWORKER_H
#define SCANWORKER_H

#include <QObject>
#include <QThread>
#include <QStringList>

class ScanWorker : public QObject
{
    Q_OBJECT

public:
    explicit ScanWorker(QObject *parent = nullptr);
    
    // 大文件扫描结果结构
    struct LargeFile {
        QStringList files;
    };
    
    // 重复文件扫描结果结构
    struct DuplicateFile {
        QString file;
        int count;
    };
    
    // 应用文件扫描结果结构
    struct AppFile {
        QString file;
        QString appName;
    };

public slots:
    // 开始扫描磁盘
    void startScan(const QString &mountPoint);
    
    // 查找大文件
    void findLargeFiles();
    
    // 查找重复文件
    void findDuplicateFiles();
    
    // 查找应用文件
    void findAppFiles();

signals:
    // 扫描完成信号
    void scanFinished();
    
    // 大文件扫描结果信号
    void largeFilesFound(const QStringList &files);
    
    // 重复文件扫描结果信号
    void duplicateFileFound(const QString &file, int count);
    
    // 应用文件扫描结果信号
    void appFileFound(const QString &file, const QString &appName);
    
    // 扫描进度信号
    void progressUpdated(int progress, const QString &message);

private:
    QString m_mountPoint;
};

#endif // SCANWORKER_H