#ifndef DISKDETAILWIDGET_H
#define DISKDETAILWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QGroupBox>

class DiskDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiskDetailWidget(QWidget *parent = nullptr);
    ~DiskDetailWidget();

    void setDiskInfo(const QString &id);
    void addBigFiles(const QStringList &files);
    void addDuplicateFile(const QString &file, int count);
    void addAppFile(const QString &file, const QString &appName);

signals:
    void backToHomeRequested();
    void scanRequested(const QString &mountPoint);
    void cleanRequested();
    void suggestionRequested();

private:
    void setupUI();

    QString currentMountPoint;

    // 顶部导航相关
    QPushButton *backButton;
    QLabel *diskNameLabel;

    // 磁盘信息相关
    QLabel *diskUsageLabel;
    QProgressBar *diskUsageProgressBar;

    // 操作按钮
    QPushButton *scanButton;
    QPushButton *cleanButton;
    QPushButton *suggestionButton;

    // 文件显示相关
    QTabWidget *tabWidget;
    QTreeWidget *fileTreeWidget;

    // 数据视图
    QTreeWidget *largeFilesWidget;
    QTreeWidget *appFilesWidget;
    QTreeWidget *duplicateFilesWidget;
};

#endif   // DISKDETAILWIDGET_H
