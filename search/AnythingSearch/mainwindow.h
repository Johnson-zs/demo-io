#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchmanager.h"
#include "resultprocessor.h"
#include "searcherinterface.h"

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
    
    // 设置搜索器
    void setSearcher(SearcherInterface *searcher);

private slots:
    void selectDirectory();
    void onSearchEditChanged(const QString &text);
    void onPathEditChanged(const QString &text);
    void onSearchResultsReady(const QStringList &results);
    void onSearchError(const QString &errorMessage);
    void onResultsProcessed(const QStringList &sortedResults);
    void showContextMenu(const QPoint &pos);
    void openContainingFolder();
    void openSelectedFile();

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
