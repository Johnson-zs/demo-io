#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchmanager.h"
#include "resultprocessor.h"

#include <QMainWindow>
#include <QStringList>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

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
    void onSearchResultsReady(const QStringList &results);
    void onSearchError(const QString &errorMessage);
    void onResultsProcessed(const QStringList &sortedResults);

private:
    void setupUI();
    void setupSearcher();
    void updateStatusLabel(const QString &message);
    void displayResults(const QStringList &results);

    QLineEdit *pathEdit;
    QLineEdit *searchEdit;
    QListWidget *resultList;
    QPushButton *browseButton;
    QLabel *statusLabel;
    
    SearchManager *searchManager { nullptr };
    ResultProcessor *resultProcessor { nullptr };
};
#endif   // MAINWINDOW_H
