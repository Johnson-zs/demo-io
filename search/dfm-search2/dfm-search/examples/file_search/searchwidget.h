#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <memory>

#include <dfm-search/search_engine.h>

class QLabel;
class QLineEdit;
class QPushButton;

/**
 * @brief 搜索界面组件类
 * 
 * 可嵌入其他界面的搜索控件
 */
class SearchWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();
    
    // 设置搜索类型
    void setSearchType(const QString &type);
    
    // 设置搜索引擎
    void setSearchEngine(std::shared_ptr<DFM::Search::SearchEngine> engine);
    
signals:
    // 搜索信号
    void searchStarted();
    void searchCompleted(const DFM::Search::SearchResult &result);
    void searchCancelled();
    void searchError(const QString &errorMessage);
    
    // 结果项选择信号
    void resultSelected(std::shared_ptr<DFM::Search::SearchResultItem> item);
    
private slots:
    // 开始搜索
    void startSearch();
    
    // 取消搜索
    void cancelSearch();
    
    // 处理搜索结果
    void handleSearchResults(const DFM::Search::SearchResult &results);
    
    // 处理搜索状态变化
    void handleSearchStatus(DFM::Search::SearchStatus status);
    
private:
    // 创建UI
    void setupUi();
    
    // UI组件
    QLabel* searchLabel_;
    QLineEdit* searchLineEdit_;
    QPushButton* searchButton_;
    QPushButton* cancelButton_;
    
    // 搜索引擎
    std::shared_ptr<DFM::Search::SearchEngine> searchEngine_;
    
    // 搜索类型
    QString searchType_;
};

#endif // SEARCHWIDGET_H 