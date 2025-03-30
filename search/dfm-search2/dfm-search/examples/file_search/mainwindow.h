#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QElapsedTimer>
#include <memory>

#include <dfm-search/dfm_search.h>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QTreeView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // 设置搜索类型
    void setSearchType(const QString &type);
    
    // 设置搜索路径
    void setSearchPath(const QString &path);
    
    // 设置是否使用索引
    void setUseIndex(bool useIndex);
    
private slots:
    // 开始搜索
    void startSearch();
    
    // 停止搜索
    void stopSearch();
    
    // 暂停/继续搜索
    void togglePause();
    
    // 选择搜索路径
    void selectSearchPath();
    
    // 处理搜索结果
    void handleSearchResults(const DFM::Search::SearchResult &results);
    
    // 处理搜索进度
    void handleSearchProgress(int percentage);
    
    // 处理搜索状态变化
    void handleSearchStatus(DFM::Search::SearchStatus status);
    
    // 处理搜索完成
    void handleSearchCompleted();
    
    // 处理搜索出错
    void handleSearchError(const QString &errorMessage);
    
    // 处理结果项双击
    void handleResultDoubleClicked(const QModelIndex &index);
    
    // 更新搜索选项
    void updateSearchOptions();
    
private:
    // 创建界面
    void setupUi();
    
    // 创建搜索引擎
    void createSearchEngine();
    
    // 更新状态栏
    void updateStatusBar(const QString &message, int timeout = 0);
    
    // UI组件
    QLabel* searchTypeLabel;
    QLineEdit* searchLineEdit;
    QPushButton* searchButton;
    QPushButton* stopButton;
    QPushButton* pauseButton;
    QLineEdit* pathLineEdit;
    QPushButton* pathButton;
    QCheckBox* caseSensitiveCheckBox;
    QCheckBox* regexCheckBox;
    QSpinBox* maxResultsSpinBox;
    QCheckBox* indexCheckBox;
    QTreeView* resultsTreeView;
    
    // 搜索引擎
    std::shared_ptr<DFM::Search::SearchEngine> searchEngine;
    
    // 搜索类型
    QString searchType;
    
    // 搜索路径
    QString searchPath;
    
    // 是否使用索引
    bool useIndex;
    
    // 搜索选项
    DFM::Search::SearchOptions searchOptions;
    
    // 搜索查询
    DFM::Search::SearchQuery searchQuery;
    
    // 状态栏标签
    QLabel *statusLabel;
    
    // 进度标签
    QLabel *progressLabel;
    
    // 搜索时间
    QElapsedTimer elapsedTimer;
};

#endif // MAINWINDOW_H 