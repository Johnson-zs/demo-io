#include "disklistwidget.h"

#include <QListWidgetItem>
#include <QStyle>
#include <QDBusInterface>
#include <QDBusReply>

#include "devicemanager.h"
#include "deviceutils.h"

DiskListWidget::DiskListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

DiskListWidget::~DiskListWidget()
{
}

void DiskListWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 标题
    titleLabel = new QLabel("磁盘列表", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // 磁盘列表
    diskList = new QListWidget(this);

    const auto idList = DeviceManager::instance().diskIdList();
    for (const auto &id : idList) {
        QListWidgetItem *diskItem = new QListWidgetItem(diskList);
        diskItem->setData(Qt::UserRole + 1, id);
        diskItem->setText(QString("%1 - %2/%3")
                                  .arg(DeviceManager::instance().idLabel(id))
                                  .arg(DeviceUtils::nameOfSize(DeviceManager::instance().sizeUsed(id)))
                                  .arg(DeviceUtils::nameOfSize(DeviceManager::instance().sizeTotal(id))));
        diskItem->setIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
    }

    diskList->setStyleSheet("QListWidget::item { padding: 15px;}");
    layout->addWidget(diskList);

    // 连接信号和槽
    connect(diskList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit diskSelected(item->data(Qt::UserRole + 1).toString());
    });
}
