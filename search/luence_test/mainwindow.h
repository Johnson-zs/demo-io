#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "searchengine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectDirectory();
    void createIndex();
    void performSearch(const QString &text);

private:
    void setupUI();
    void scanDirectory(const QString &path, QStringList &fileList);

    QLineEdit *pathEdit;
    QLineEdit *searchEdit;
    QListWidget *resultList;
    QPushButton *browseButton;
    QPushButton *indexButton;
    
    QStringList fileList;
    SearchEngine searchEngine;
};

#endif // MAINWINDOW_H 