#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QTreeView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QGroupBox>
#include <memory>

// 包含搜索引擎接口头文件
#include "dfm-search/searchengine.h"

class QHBoxLayout;
class QVBoxLayout;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(std::shared_ptr<DFM::Search::ISearchEngine> engine, QWidget *parent = nullptr);
    ~SearchWidget();

    // 设置搜索范围
    void setScope(const DFM::Search::SearchScope& scope);
    
    // 清除搜索
    void clearSearch();

private slots:
    // 搜索操作
    void onSearch();
    void onCancel();
    void onPause();
    void onResume();
    
    // 选项变更
    void onOptionsChanged();
    
    // 搜索事件处理
    void onResultsReady(const DFM::Search::SearchResultSet& results);
    void onProgressChanged(const DFM::Search::SearchProgress& progress);
    void onStateChanged(DFM::Search::SearchState state);
    void onSearchCompleted(bool success);
    void onErrorOccurred(const QString& error);
    
    // 结果处理
    void onResultItemActivated(const QModelIndex& index);
    void onCustomizeColumns();
    
    // 路径选择
    void onSelectSearchPath();

private:
    // UI 设置
    void setupUI();
    void setupSearchOptions();
    void setupResultsView();
    void updateUI();
    
    // 搜索操作
    void startSearch();
    void resetState();
    
    // 结果处理
    void clearResults();
    void addResult(const std::shared_ptr<DFM::Search::ISearchResult>& result);
    void updateProgressInfo(const DFM::Search::SearchProgress& progress);
    
    // 结果回调处理
    void onResultCallback(const std::shared_ptr<DFM::Search::ISearchResult>& result);

private:
    // 搜索引擎
    std::shared_ptr<DFM::Search::ISearchEngine> m_engine;
    
    // 搜索状态
    bool m_isSearching;
    
    // 搜索输入
    QString m_currentQuery;
    
    // UI 组件
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QPushButton *m_cancelButton;
    QPushButton *m_pauseButton;
    
    // 搜索选项
    QGroupBox *m_optionsGroupBox;
    QCheckBox *m_caseSensitiveCheck;
    QCheckBox *m_recursiveCheck;
    QCheckBox *m_regexCheck;
    QCheckBox *m_hiddenFilesCheck;
    QCheckBox *m_followSymlinksCheck;
    
    // 搜索范围
    QLineEdit *m_pathEdit;
    QToolButton *m_pathButton;
    QLineEdit *m_includeFilterEdit;
    QLineEdit *m_excludeFilterEdit;
    
    // 进度指示
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QLabel *m_progressInfoLabel;
    
    // 结果视图
    QTreeView *m_resultsView;
    QStandardItemModel *m_resultModel;
    QSortFilterProxyModel *m_proxyModel;
    
    // 布局
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_searchLayout;
    QHBoxLayout *m_progressLayout;
};

#endif // SEARCHWIDGET_H 