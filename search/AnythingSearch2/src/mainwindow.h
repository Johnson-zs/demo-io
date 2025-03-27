#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDir>
#include "filelistmodel.h"
#include "fileitemdelegate.h"
#include "searchmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectSearchPath();
    void onSearchTextChanged(const QString &text);
    void performSearch();

private:
    void setupUI();
    
    QPushButton *m_pathButton;
    QLineEdit *m_searchLineEdit;
    QListView *m_fileListView;
    FileListModel *m_fileModel;
    FileItemDelegate *m_fileDelegate;
    SearchManager *m_searchManager;
    QTimer *m_searchDebounceTimer;
    QString m_currentPath;
};

#endif // MAINWINDOW_H 