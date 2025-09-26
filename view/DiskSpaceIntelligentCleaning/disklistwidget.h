#ifndef DISKLISTWIDGET_H
#define DISKLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

class DiskListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiskListWidget(QWidget *parent = nullptr);
    ~DiskListWidget();

signals:
    void diskSelected(const QString id);

private:
    void setupUI();

    QListWidget *diskList;
    QLabel *titleLabel;
};

#endif   // DISKLISTWIDGET_H
