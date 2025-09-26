#include "diskdetailwidget.h"

#include <QStyle>
#include <QFileInfo>

#include "devicemanager.h"
#include "deviceutils.h"

DiskDetailWidget::DiskDetailWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

DiskDetailWidget::~DiskDetailWidget()
{
}

void DiskDetailWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 顶部导航栏
    QHBoxLayout *topLayout = new QHBoxLayout();

    // 返回首页按钮
    backButton = new QPushButton("返回首页", this);
    backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    topLayout->addWidget(backButton);

    // 磁盘名称标签
    diskNameLabel = new QLabel("系统盘", this);
    diskNameLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    topLayout->addWidget(diskNameLabel);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 磁盘使用情况信息
    QGroupBox *usageGroup = new QGroupBox("磁盘使用情况", this);
    QVBoxLayout *usageLayout = new QVBoxLayout(usageGroup);

    diskUsageLabel = new QLabel("已使用:  / 总计: (%) )", this);
    usageLayout->addWidget(diskUsageLabel);

    diskUsageProgressBar = new QProgressBar(this);
    diskUsageProgressBar->setRange(0, 100);
    diskUsageProgressBar->setValue(0);
    usageLayout->addWidget(diskUsageProgressBar);

    mainLayout->addWidget(usageGroup);

    // 创建操作按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 扫描按钮
    scanButton = new QPushButton("扫描磁盘", this);
    scanButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    buttonLayout->addWidget(scanButton);

    // 清理建议按钮
    suggestionButton = new QPushButton("获取清理建议", this);
    suggestionButton->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    buttonLayout->addWidget(suggestionButton);

    // 清理按钮
    cleanButton = new QPushButton("执行清理", this);
    cleanButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    buttonLayout->addWidget(cleanButton);

    mainLayout->addLayout(buttonLayout);

    // 创建标签页控件
    tabWidget = new QTabWidget(this);

    // 创建大文件标签页
    largeFilesWidget = new QTreeWidget(this);
    largeFilesWidget->setHeaderLabels(QStringList() << "文件名"
                                                    << "路径"
                                                    << "大小");
    tabWidget->addTab(largeFilesWidget, "大文件 (>50MB)");

    // 创建应用文件标签页
    appFilesWidget = new QTreeWidget(this);
    appFilesWidget->setHeaderLabels(QStringList() << "文件名"
                                                  << "路径"
                                                  << "应用"
                                                  << "大小");
    tabWidget->addTab(appFilesWidget, "应用文件");

    // 创建重复文件标签页
    duplicateFilesWidget = new QTreeWidget(this);
    duplicateFilesWidget->setHeaderLabels(QStringList() << "文件名"
                                                        << "路径"
                                                        << "大小"
                                                        << "重复次数");
    tabWidget->addTab(duplicateFilesWidget, "重复文件");

    mainLayout->addWidget(tabWidget);

    // 连接信号和槽
    connect(backButton, &QPushButton::clicked, this, &DiskDetailWidget::backToHomeRequested);
    connect(scanButton, &QPushButton::clicked, this, [this]() {
        emit scanRequested(currentMountPoint);
    });
    connect(cleanButton, &QPushButton::clicked, this, &DiskDetailWidget::cleanRequested);
    connect(suggestionButton, &QPushButton::clicked, this, &DiskDetailWidget::suggestionRequested);
}

void DiskDetailWidget::setDiskInfo(const QString &id)
{
    currentMountPoint = DeviceManager::instance().mountPoint(id);
    // 数据更新
    auto used = DeviceManager::instance().sizeUsed(id);
    auto total = DeviceManager::instance().sizeTotal(id);
    auto displayUsed = DeviceUtils::nameOfSize(used);
    auto displayTotal = DeviceUtils::nameOfSize(total);
    int percent = qFloor(qreal(used) * 100 / total);
    diskNameLabel->setText(DeviceManager::instance().idLabel(id));
    diskUsageLabel->setText(QString("已使用: %1 / 总计: %2 (%3 %) )").arg(displayUsed).arg(displayTotal).arg(percent));
    diskUsageProgressBar->setValue(percent);

    // 状态重置
    tabWidget->setCurrentIndex(0);
    largeFilesWidget->clear();
    appFilesWidget->clear();
    duplicateFilesWidget->clear();
}

void DiskDetailWidget::addBigFiles(const QStringList &files)
{
    for (const QString &file : files) {
        QTreeWidgetItem *item = new QTreeWidgetItem(largeFilesWidget);
        QFileInfo fileInfo(file);

        item->setText(0, fileInfo.fileName());
        item->setText(1, fileInfo.absolutePath());
        item->setText(2, DeviceUtils::nameOfSize(fileInfo.size()));
        item->setCheckState(0, Qt::Unchecked);

        largeFilesWidget->addTopLevelItem(item);
    }
}

void DiskDetailWidget::addDuplicateFile(const QString &file, int count)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(duplicateFilesWidget);
    QFileInfo fileInfo(file);

    item->setText(0, fileInfo.fileName());
    item->setText(1, fileInfo.absolutePath());
    item->setText(2, DeviceUtils::nameOfSize(fileInfo.size()));
    item->setText(3, QString::number(count));
    item->setCheckState(0, Qt::Unchecked);

    duplicateFilesWidget->addTopLevelItem(item);
}

void DiskDetailWidget::addAppFile(const QString &file, const QString &appName)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(appFilesWidget);
    QFileInfo fileInfo(file);

    item->setText(0, fileInfo.fileName());
    item->setText(1, fileInfo.absolutePath());
    item->setText(2, appName);
    item->setText(3, DeviceUtils::nameOfSize(fileInfo.size()));
    item->setCheckState(0, Qt::Unchecked);

    appFilesWidget->addTopLevelItem(item);
}
