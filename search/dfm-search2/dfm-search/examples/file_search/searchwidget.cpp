#include "searchwidget.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
    , searchEngine_(nullptr)
    , searchType_("filename")
{
    setupUi();
}

void SearchWidget::setupUi()
{
    // 创建主布局
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建搜索标签
    searchLabel_ = new QLabel("搜索：", this);
    layout->addWidget(searchLabel_);
    
    // 创建搜索输入框
    searchLineEdit_ = new QLineEdit(this);
    searchLineEdit_->setPlaceholderText("输入搜索关键词");
    searchLineEdit_->setClearButtonEnabled(true);
    connect(searchLineEdit_, &QLineEdit::returnPressed, this, &SearchWidget::startSearch);
    layout->addWidget(searchLineEdit_, 1);
    
    // 创建搜索按钮
    searchButton_ = new QPushButton("搜索", this);
    searchButton_->setIcon(QIcon::fromTheme("search"));
    connect(searchButton_, &QPushButton::clicked, this, &SearchWidget::startSearch);
    layout->addWidget(searchButton_);
    
    // 创建取消按钮
    cancelButton_ = new QPushButton("取消", this);
    cancelButton_->setIcon(QIcon::fromTheme("process-stop"));
    cancelButton_->setEnabled(false);
    connect(cancelButton_, &QPushButton::clicked, this, &SearchWidget::cancelSearch);
    layout->addWidget(cancelButton_);
    
    // 应用默认搜索类型
    setSearchType(searchType_);
}

SearchWidget::~SearchWidget()
{
    // 所有的子部件会自动销毁
}

void SearchWidget::setSearchType(const QString &type)
{
    searchType_ = type.toLower();
    
    // 更新UI
    if (searchType_ == "filename") {
        searchLabel_->setText("文件名：");
        searchLineEdit_->setPlaceholderText("输入文件名进行搜索");
    } else if (searchType_ == "content") {
        searchLabel_->setText("内容：");
        searchLineEdit_->setPlaceholderText("输入文件内容进行搜索");
    } else if (searchType_ == "app") {
        searchLabel_->setText("应用：");
        searchLineEdit_->setPlaceholderText("输入应用名称进行搜索");
    } else {
        searchLabel_->setText("搜索：");
        searchLineEdit_->setPlaceholderText("输入搜索关键词");
    }
}

void SearchWidget::setSearchEngine(std::shared_ptr<DFM::Search::SearchEngine> engine)
{
    // 断开之前的引擎连接
    if (searchEngine_) {
        searchEngine_->disconnect(this);
    }
    
    searchEngine_ = engine;
    
    // 连接新引擎的信号
    if (searchEngine_) {
        connect(searchEngine_.get(), &DFM::Search::SearchEngine::resultsReady,
                this, &SearchWidget::handleSearchResults);
        connect(searchEngine_.get(), &DFM::Search::SearchEngine::statusChanged,
                this, &SearchWidget::handleSearchStatus);
        connect(searchEngine_.get(), &DFM::Search::SearchEngine::searchCompleted,
                this, [this]() {
                    emit searchCompleted(searchEngine_->getResults());
                });
        connect(searchEngine_.get(), &DFM::Search::SearchEngine::searchCancelled,
                this, &SearchWidget::searchCancelled);
        connect(searchEngine_.get(), &DFM::Search::SearchEngine::searchError,
                this, &SearchWidget::searchError);
    }
}

void SearchWidget::startSearch()
{
    // 获取查询字符串
    QString query = searchLineEdit_->text();
    if (query.isEmpty()) {
        return;
    }
    
    // 检查搜索引擎是否存在
    if (!searchEngine_) {
        qWarning() << "搜索引擎未设置";
        emit searchError("搜索引擎未设置");
        return;
    }
    
    // 创建搜索查询
    DFM::Search::SearchQuery searchQuery;
    if (searchType_ == "filename") {
        searchQuery = DFM::Search::SearchQuery::createFilenameQuery(query);
    } else if (searchType_ == "content") {
        searchQuery = DFM::Search::SearchQuery::createContentQuery(query);
    } else if (searchType_ == "app") {
        searchQuery = DFM::Search::SearchQuery::createAppQuery(query);
    } else {
        searchQuery.setQueryString(query);
    }
    
    // 更新UI状态
    searchButton_->setEnabled(false);
    cancelButton_->setEnabled(true);
    
    // 开始搜索
    if (searchEngine_->search(searchQuery)) {
        emit searchStarted();
    } else {
        searchButton_->setEnabled(true);
        cancelButton_->setEnabled(false);
        emit searchError("无法启动搜索");
    }
}

void SearchWidget::cancelSearch()
{
    if (searchEngine_) {
        searchEngine_->cancel();
    }
}

void SearchWidget::handleSearchResults(const DFM::Search::SearchResult &results)
{
    // 这里仅转发信号
    emit searchCompleted(results);
}

void SearchWidget::handleSearchStatus(DFM::Search::SearchStatus status)
{
    switch (status) {
        case DFM::Search::SearchStatus::Ready:
        case DFM::Search::SearchStatus::Completed:
        case DFM::Search::SearchStatus::Cancelled:
        case DFM::Search::SearchStatus::Error:
            // 搜索结束，恢复UI状态
            searchButton_->setEnabled(true);
            cancelButton_->setEnabled(false);
            break;
            
        case DFM::Search::SearchStatus::Running:
            // 搜索中，更新UI状态
            searchButton_->setEnabled(false);
            cancelButton_->setEnabled(true);
            break;
            
        case DFM::Search::SearchStatus::Paused:
            // 搜索暂停，暂不处理
            break;
    }
}