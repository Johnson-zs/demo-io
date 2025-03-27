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
#include <QScrollBar>
#include <QStatusBar>
#include <QLabel>
#include <QCheckBox>
#include <QMenu>
#include <QAction>

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
    void onSearchStatusChanged(SearchManager::SearchStatus status, const QString &message);
    void onSearchOptionChanged();
    void onSearchResultsReady(const QVector<FileData> &results);
    void showContextMenu(const QPoint &pos);
    void openContainingFolder();
    void openSelectedFile();
    void copyFilePath();
    void showSearchHelp();

private:
    void setupUI();
    void setupStatusBar();
    void updateStatusBar(int count);
    
    QPushButton *m_pathButton;
    QLineEdit *m_searchLineEdit;
    QListView *m_fileListView;
    FileListModel *m_fileModel;
    FileItemDelegate *m_fileDelegate;
    SearchManager *m_searchManager;
    QTimer *m_searchDebounceTimer;
    QString m_currentPath;
    QString m_currentSearchKeyword;
    int m_currentBatchSize;
    int m_currentOffset;
    QScrollBar *m_scrollBar;
    QStatusBar *m_statusBar;
    QLabel *m_statusLabel;
    
    QCheckBox *m_caseSensitiveCheckBox;
    QCheckBox *m_fuzzySearchCheckBox;
    QHBoxLayout *m_searchOptionsLayout;
    
    void loadMoreResults();
    bool m_isLoadingMore = false;
    
    QMenu *m_contextMenu;
    QAction *m_openFolderAction;
    QAction *m_openFileAction;
    QAction *m_copyPathAction;
};

#endif // MAINWINDOW_H 