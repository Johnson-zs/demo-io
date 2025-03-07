#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "anythingsearcher.h"

#include <QMainWindow>
#include <QStringList>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectDirectory();
    void onSearchEditChanged(const QString &text);
    void onPathEditChanged(const QString &text);

private:
    void setupUI();
    void setupSearcher();
    void performSearch(const QString &text);

    QLineEdit *pathEdit;
    QLineEdit *searchEdit;
    QListWidget *resultList;
    QPushButton *browseButton;
    AnythingSearcher *searcher { nullptr };
};
#endif   // MAINWINDOW_H
