#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <memory>

#include "dfm-search/searchengine.h"
#include "dfm-search/filenamesearch.h"

namespace Ui {
class SearchWidget;
}

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();

    void setSearchType(DFM::Search::SearchType type);
    void setSearchMode(DFM::Search::SearchMode mode);

public slots:
    void onSearch();
    void onCancel();
    void onPause();
    void onResume();
    void onClear();
    void onOptionsChanged();

private slots:
    void onResultsReady(const DFM::Search::SearchResultSet& results);
    void onProgressChanged(const DFM::Search::SearchProgress& progress);
    void onStateChanged(DFM::Search::SearchState state);
    void onSearchCompleted(bool success);
    void onErrorOccurred(const QString& error);
    void onResultItemActivated(const QModelIndex& index);

private:
    void setupUi();
    void updateUi();
    void setupSearchOptions();
    void setupResultsView();
    void clearResults();
    void addResult(const DFM::Search::SearchResult& result);
    void updateProgressInfo(const DFM::Search::SearchProgress& progress);
    void resetSearchEngine();
    
    // 获取当前搜索选项
    DFM::Search::SearchOptions getSearchOptions() const;

private:
    Ui::SearchWidget *ui;
    std::shared_ptr<DFM::Search::ISearchEngine> m_searchEngine;
    QStandardItemModel *m_resultModel;
    QSortFilterProxyModel *m_proxyModel;
    DFM::Search::SearchType m_searchType;
    DFM::Search::SearchMode m_searchMode;
    QString m_currentQuery;
    bool m_isSearching;
};

#endif // SEARCHWIDGET_H 