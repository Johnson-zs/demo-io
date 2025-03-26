#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QTreeView>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QCheckBox>
#include <QTabWidget>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <qsearch/search_manager.h>
#include <qsearch/search_query.h>
#include <qsearch/index_manager.h>

class AdvancedSearchWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AdvancedSearchWindow(QWidget *parent = nullptr);
    ~AdvancedSearchWindow();
    
    void setUseWorker(bool useWorker);
    void setIndexPath(const QString& path);
    
private slots:
    void onSearch();
    void onClear();
    void onIndexSelected(int index);
    void onTabChanged(int index);
    void onCreateIndex();
    void onUpdateIndex();
    void onSearchProgressChanged(int searchId, double progress);
    void onResultItemFound(int searchId, const QSearch::ResultItem& item);
    void onSearchCompleted(int searchId);
    void onSearchError(int searchId, const QString& error);
    
private:
    void setupUI();
    void setupConnections();
    QSearch::SearchQuery buildQuery();
    QSearch::SearchOptions buildOptions();
    void displayResult(const QSearch::ResultItem& item);
    void updateIndexStatus();
    void showStatusMessage(const QString& message, bool isError = false);
    
    // UI 组件
    QLineEdit* searchTextEdit;
    QComboBox* searchTypeCombo;
    QComboBox* matchTypeCombo;
    QComboBox* sortOrderCombo;
    QTreeView* resultsTreeView;
    QPushButton* searchButton;
    QPushButton* clearButton;
    QProgressBar* progressBar;
    QLabel* statusLabel;
    QTabWidget* mainTabWidget;
    
    // 高级搜索选项
    QLineEdit* pathEdit;
    QPushButton* browseButton;
    QCheckBox* recursiveCheckBox;
    QCheckBox* hiddenFilesCheckBox;
    QDateEdit* fromDateEdit;
    QDateEdit* toDateEdit;
    QSpinBox* minSizeSpinBox;
    QSpinBox* maxSizeSpinBox;
    QComboBox* sizeUnitCombo;
    QLineEdit* extensionsEdit;
    QCheckBox* caseSensitiveCheckBox;
    QSpinBox* maxResultsSpinBox;
    QSpinBox* timeoutSpinBox;
    
    // 索引选项
    QComboBox* indexTypeCombo;
    QPushButton* createIndexButton;
    QPushButton* updateIndexButton;
    QLabel* indexStatusLabel;
    QProgressBar* indexProgressBar;
    
    // 数据模型
    QStandardItemModel* resultsModel;
    QSortFilterProxyModel* proxyModel;
    
    // 搜索管理
    QSearch::SearchManager searchManager;
    QSearch::IndexManager* indexManager;
    int currentSearchId;
    bool useWorkerMode;
    QString indexPath;
}; 