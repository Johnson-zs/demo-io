#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QVBoxLayout>
#include <QWidget>

class FileViewModel;
class FileViewDelegate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void populateTestData();
    
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QListView *listView;
    FileViewModel *model;
    FileViewDelegate *delegate;
};

#endif // MAINWINDOW_H
