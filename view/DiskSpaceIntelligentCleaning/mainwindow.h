#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QThread>

#include "diskdetailwidget.h"
#include "disklistwidget.h"
#include "scanworker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 扫描磁盘使用情况
    void scanDiskUsage(const QString &mountPoint);

    // 识别大文件
    void detectLargeFiles();

    // 识别应用产生的文件
    void detectAppFiles();

    // 识别重复文件
    void detectDuplicateFiles();

    // 提供清理建议
    void provideCleaningSuggestions();

    // 执行清理操作
    void performCleaning();

    // 返回首页
    void backToHome();

    // 进入磁盘详情
    void enterDiskDetail(const QString id);
    
    // 处理扫描完成事件
    void onScanFinished();
    
    // 处理大文件扫描结果
    void onLargeFilesFound(const QStringList &files);
    
    // 处理重复文件扫描结果
    void onDuplicateFileFound(const QString &file, int count);
    
    // 处理应用文件扫描结果
    void onAppFileFound(const QString &file, const QString &appName);
    
    // 处理扫描进度更新
    void onProgressUpdated(int progress, const QString &message);

private:
    // 初始化界面
    void setupUI();

    // 更新磁盘使用可视化
    void updateDiskVisualization();

    // 页面切换控件
    QStackedWidget *stackedWidget;

    // 页面组件
    DiskListWidget *diskListWidget;
    DiskDetailWidget *diskDetailWidget;
    
    // 扫描线程相关
    QThread *scanThread;
    ScanWorker *scanWorker;
};
#endif   // MAINWINDOW_H