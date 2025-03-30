#include "searchwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDesktopServices>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDateTime>
#include <QUrl>
#include <QDebug>

using namespace DFM::Search;

SearchWidget::SearchWidget(std::shared_ptr<ISearchEngine> engine, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
    , m_isSearching(false)
    , m_searchEdit(nullptr)
    , m_searchButton(nullptr)
    , m_cancelButton(nullptr)
    , m_pauseButton(nullptr)
    , m_optionsGroupBox(nullptr)
    , m_caseSensitiveCheck(nullptr)
    , m_recursiveCheck(nullptr)
    , m_regexCheck(nullptr)
    , m_hiddenFilesCheck(nullptr)
    , m_followSymlinksCheck(nullptr)
    , m_pathEdit(nullptr)
    , m_pathButton(nullptr)
    , m_includeFilterEdit(nullptr)
    , m_excludeFilterEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_progressInfoLabel(nullptr)
    , m_resultsView(nullptr)
    , m_resultModel(nullptr)
    , m_proxyModel(nullptr)
    , m_mainLayout(nullptr)
    , m_searchLayout(nullptr)
    , m_progressLayout(nullptr)
{
    setupUI();
    
    // 连接搜索引擎信号
    if (m_engine) {
        connect(m_engine.get(), &ISearchEngine::resultsReady,
                this, &SearchWidget::onResultsReady);
        connect(m_engine.get(), &ISearchEngine::progressChanged,
                this, &SearchWidget::onProgressChanged);
        connect(m_engine.get(), &ISearchEngine::stateChanged,
                this, &SearchWidget::onStateChanged);
        connect(m_engine.get(), &ISearchEngine::searchCompleted,
                this, &SearchWidget::onSearchCompleted);
        connect(m_engine.get(), &ISearchEngine::errorOccurred,
                this, &SearchWidget::onErrorOccurred);
        
        // 设置回调
        m_engine->setResultCallback([this](const std::shared_ptr<ISearchResult>& result) {
            onResultCallback(result);
        });
    }
}

SearchWidget::~SearchWidget()
{
    // 确保停止搜索
    if (m_engine && m_isSearching) {
        m_engine->cancelSearch();
    }
}

void SearchWidget::setScope(const SearchScope& scope)
{
    if (m_engine) {
        m_engine->setScope(scope);
        
        // 更新UI
        if (!scope.includePaths.isEmpty()) {
            m_pathEdit->setText(scope.includePaths.join(";"));
        }
        
        if (!scope.includePatterns.isEmpty()) {
            m_includeFilterEdit->setText(scope.includePatterns.join(";"));
        }
        
        if (!scope.excludePatterns.isEmpty()) {
            m_excludeFilterEdit->setText(scope.excludePatterns.join(";"));
        }
    }
}

void SearchWidget::clearSearch()
{
    // 取消当前搜索
    if (m_isSearching) {
        onCancel();
    }
    
    // 清除结果和状态
    clearResults();
    m_searchEdit->clear();
    updateUI();
}

void SearchWidget::onSearch()
{
    m_currentQuery = m_searchEdit->text().trimmed();
    if (m_currentQuery.isEmpty()) {
        QMessageBox::warning(this, "搜索错误", "请输入搜索查询");
        return;
    }
    
    // 应用选项
    onOptionsChanged();
    
    // 开始搜索
    startSearch();
}

void SearchWidget::onCancel()
{
    if (m_engine && m_isSearching) {
        m_engine->cancelSearch();
    }
}

void SearchWidget::onPause()
{
    if (m_engine && m_isSearching) {
        m_engine->pauseSearch();
        updateUI();
    }
}

void SearchWidget::onResume()
{
    if (m_engine && m_engine->state() == SearchState::Paused) {
        m_engine->resumeSearch();
        updateUI();
    }
}

void SearchWidget::onOptionsChanged()
{
    if (!m_engine) {
        return;
    }
    
    // 收集搜索选项
    SearchOptions options = SearchOptionFlag::None;
    
    if (m_caseSensitiveCheck->isChecked()) {
        options |= SearchOptionFlag::CaseSensitive;
    }
    
    if (m_recursiveCheck->isChecked()) {
        options |= SearchOptionFlag::RecursiveSearch;
    }
    
    if (m_regexCheck->isChecked()) {
        options |= SearchOptionFlag::RegexPattern;
    }
    
    if (m_hiddenFilesCheck->isChecked()) {
        options |= SearchOptionFlag::HiddenFiles;
    }
    
    if (m_followSymlinksCheck->isChecked()) {
        options |= SearchOptionFlag::FollowSymlinks;
    }
    
    // 设置选项
    m_engine->setOptions(options);
    
    // 收集搜索范围
    SearchScope scope;
    
    // 包含路径
    QString pathText = m_pathEdit->text().trimmed();
    if (!pathText.isEmpty()) {
        scope.includePaths = pathText.split(";", Qt::SkipEmptyParts);
    }
    
    // 包含模式
    QString includeFilter = m_includeFilterEdit->text().trimmed();
    if (!includeFilter.isEmpty()) {
        scope.includePatterns = includeFilter.split(";", Qt::SkipEmptyParts);
    }
    
    // 排除模式
    QString excludeFilter = m_excludeFilterEdit->text().trimmed();
    if (!excludeFilter.isEmpty()) {
        scope.excludePatterns = excludeFilter.split(";", Qt::SkipEmptyParts);
    }
    
    // 设置范围
    m_engine->setScope(scope);
}

void SearchWidget::onResultsReady(const SearchResultSet& results)
{
    // 将新结果添加到模型
    const auto& resultItems = results.results();
    for (const auto& result : resultItems) {
        addResult(result);
    }
}

void SearchWidget::onProgressChanged(const SearchProgress& progress)
{
    updateProgressInfo(progress);
}

void SearchWidget::onStateChanged(SearchState state)
{
    switch (state) {
        case SearchState::Idle:
            m_statusLabel->setText("就绪");
            m_isSearching = false;
            break;
        
        case SearchState::Preparing:
            m_statusLabel->setText("准备中...");
            m_isSearching = true;
            break;
        
        case SearchState::Searching:
            m_statusLabel->setText("搜索中...");
            m_isSearching = true;
            break;
        
        case SearchState::Paused:
            m_statusLabel->setText("已暂停");
            break;
        
        case SearchState::Completed:
            m_statusLabel->setText("已完成");
            m_isSearching = false;
            break;
        
        case SearchState::Cancelled:
            m_statusLabel->setText("已取消");
            m_isSearching = false;
            break;
        
        case SearchState::Error:
            m_statusLabel->setText("出错");
            m_isSearching = false;
            break;
        
        default:
            break;
    }
    
    updateUI();
}

void SearchWidget::onSearchCompleted(bool success)
{
    m_isSearching = false;
    if (success) {
        m_statusLabel->setText(QString("已完成 (%1 个结果)").arg(m_resultModel->rowCount()));
    } else {
        m_statusLabel->setText("搜索未完成");
    }
    updateUI();
}

void SearchWidget::onErrorOccurred(const QString& error)
{
    m_statusLabel->setText("错误: " + error);
    m_isSearching = false;
    updateUI();
}

void SearchWidget::onResultItemActivated(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    
    // 获取源模型索引
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (!sourceIndex.isValid()) {
        return;
    }
    
    // 获取文件路径
    QString filePath = m_resultModel->data(
        m_resultModel->index(sourceIndex.row(), 0), Qt::UserRole).toString();
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 打开文件
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir()) {
        // 打开文件夹
        QUrl url = QUrl::fromLocalFile(filePath);
        QDesktopServices::openUrl(url);
    } else {
        // 打开文件
        QUrl url = QUrl::fromLocalFile(filePath);
        QDesktopServices::openUrl(url);
    }
}

void SearchWidget::onCustomizeColumns()
{
    // 打开一个对话框，允许用户选择要显示的列
    QMenu menu(this);
    
    for (int i = 0; i < m_resultModel->columnCount(); ++i) {
        QString columnName = m_resultModel->headerData(i, Qt::Horizontal).toString();
        QAction *action = menu.addAction(columnName);
        action->setCheckable(true);
        action->setChecked(!m_resultsView->isColumnHidden(i));
        
        connect(action, &QAction::toggled, [this, i](bool checked) {
            m_resultsView->setColumnHidden(i, !checked);
        });
    }
    
    menu.exec(QCursor::pos());
}

void SearchWidget::onSelectSearchPath()
{
    QString currentPath = m_pathEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QDir::homePath();
    }
    
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择搜索目录", currentPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void SearchWidget::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建搜索输入区域
    m_searchLayout = new QHBoxLayout();
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("输入搜索查询...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SearchWidget::onSearch);
    
    m_searchButton = new QPushButton("搜索", this);
    connect(m_searchButton, &QPushButton::clicked, this, &SearchWidget::onSearch);
    
    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &SearchWidget::onCancel);
    
    m_pauseButton = new QPushButton("暂停", this);
    connect(m_pauseButton, &QPushButton::clicked, this, &SearchWidget::onPause);
    
    m_searchLayout->addWidget(m_searchEdit, 1);
    m_searchLayout->addWidget(m_searchButton);
    m_searchLayout->addWidget(m_pauseButton);
    m_searchLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_searchLayout);
    
    // 创建搜索选项
    setupSearchOptions();
    
    // 创建进度区域
    m_progressLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel("就绪", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    
    m_progressInfoLabel = new QLabel(this);
    
    m_progressLayout->addWidget(m_statusLabel);
    m_progressLayout->addWidget(m_progressBar);
    m_progressLayout->addWidget(m_progressInfoLabel);
    
    m_mainLayout->addLayout(m_progressLayout);
    
    // 创建结果视图
    setupResultsView();
    
    // 初始状态设置
    updateUI();
}

void SearchWidget::setupSearchOptions()
{
    // 创建选项框架
    m_optionsGroupBox = new QGroupBox("搜索选项", this);
    QGridLayout *optionsLayout = new QGridLayout(m_optionsGroupBox);
    
    // 创建选项控件
    m_caseSensitiveCheck = new QCheckBox("区分大小写", m_optionsGroupBox);
    m_recursiveCheck = new QCheckBox("递归搜索子目录", m_optionsGroupBox);
    m_regexCheck = new QCheckBox("使用正则表达式", m_optionsGroupBox);
    m_hiddenFilesCheck = new QCheckBox("包含隐藏文件", m_optionsGroupBox);
    m_followSymlinksCheck = new QCheckBox("跟随符号链接", m_optionsGroupBox);
    
    // 默认选项
    m_recursiveCheck->setChecked(true);
    
    // 添加到布局
    optionsLayout->addWidget(m_caseSensitiveCheck, 0, 0);
    optionsLayout->addWidget(m_recursiveCheck, 0, 1);
    optionsLayout->addWidget(m_regexCheck, 0, 2);
    optionsLayout->addWidget(m_hiddenFilesCheck, 0, 3);
    optionsLayout->addWidget(m_followSymlinksCheck, 0, 4);
    
    // 创建路径设置
    QLabel *pathLabel = new QLabel("搜索路径:", m_optionsGroupBox);
    m_pathEdit = new QLineEdit(m_optionsGroupBox);
    m_pathButton = new QToolButton(m_optionsGroupBox);
    m_pathButton->setText("...");
    connect(m_pathButton, &QToolButton::clicked, this, &SearchWidget::onSelectSearchPath);
    
    optionsLayout->addWidget(pathLabel, 1, 0);
    optionsLayout->addWidget(m_pathEdit, 1, 1, 1, 3);
    optionsLayout->addWidget(m_pathButton, 1, 4);
    
    // 创建过滤器设置
    QLabel *includeLabel = new QLabel("包含过滤器:", m_optionsGroupBox);
    m_includeFilterEdit = new QLineEdit(m_optionsGroupBox);
    m_includeFilterEdit->setPlaceholderText("*.txt;*.cpp;*.h");
    
    QLabel *excludeLabel = new QLabel("排除过滤器:", m_optionsGroupBox);
    m_excludeFilterEdit = new QLineEdit(m_optionsGroupBox);
    m_excludeFilterEdit->setPlaceholderText("*.tmp;*.bak");
    
    optionsLayout->addWidget(includeLabel, 2, 0);
    optionsLayout->addWidget(m_includeFilterEdit, 2, 1, 1, 1);
    optionsLayout->addWidget(excludeLabel, 2, 2);
    optionsLayout->addWidget(m_excludeFilterEdit, 2, 3, 1, 2);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_optionsGroupBox);
    
    // 连接选项变更信号
    connect(m_caseSensitiveCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(m_recursiveCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(m_regexCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(m_hiddenFilesCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(m_followSymlinksCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
}

void SearchWidget::setupResultsView()
{
    // 创建结果视图
    m_resultsView = new QTreeView(this);
    m_resultsView->setRootIsDecorated(false);
    m_resultsView->setAlternatingRowColors(true);
    m_resultsView->setSortingEnabled(true);
    m_resultsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_resultsView->setMinimumHeight(200);
    
    // 创建模型
    m_resultModel = new QStandardItemModel(this);
    m_resultModel->setColumnCount(5);
    m_resultModel->setHorizontalHeaderLabels(QStringList() << "名称" << "路径" << "大小" << "修改时间" << "类型");
    
    // 创建代理模型进行排序和过滤
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_resultModel);
    m_resultsView->setModel(m_proxyModel);
    
    // 设置列宽
    m_resultsView->header()->setSectionResizeMode(QHeaderView::Interactive);
    m_resultsView->header()->setStretchLastSection(true);
    m_resultsView->header()->setSortIndicator(0, Qt::AscendingOrder);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_resultsView, 1);
    
    // 连接信号
    connect(m_resultsView, &QTreeView::activated, this, &SearchWidget::onResultItemActivated);
    connect(m_resultsView, &QTreeView::customContextMenuRequested, [this](const QPoint& pos) {
        QModelIndex index = m_resultsView->indexAt(pos);
        if (index.isValid()) {
            QMenu menu(this);
            
            QAction *openAction = menu.addAction("打开");
            connect(openAction, &QAction::triggered, [this, index]() {
                onResultItemActivated(index);
            });
            
            QAction *openFolderAction = menu.addAction("打开所在文件夹");
            connect(openFolderAction, &QAction::triggered, [this, index]() {
                // 获取源模型索引
                QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
                if (!sourceIndex.isValid()) {
                    return;
                }
                
                // 获取文件路径
                QString filePath = m_resultModel->data(
                    m_resultModel->index(sourceIndex.row(), 0), Qt::UserRole).toString();
                
                if (!filePath.isEmpty()) {
                    // 打开文件夹
                    QFileInfo fileInfo(filePath);
                    QUrl url = QUrl::fromLocalFile(fileInfo.path());
                    QDesktopServices::openUrl(url);
                }
            });
            
            menu.addSeparator();
            
            QAction *customizeColumnsAction = menu.addAction("自定义列...");
            connect(customizeColumnsAction, &QAction::triggered, this, &SearchWidget::onCustomizeColumns);
            
            menu.exec(m_resultsView->mapToGlobal(pos));
        }
    });
}

void SearchWidget::updateUI()
{
    bool isSearching = m_isSearching;
    bool isPaused = m_engine && m_engine->state() == SearchState::Paused;
    
    // 更新按钮状态
    m_searchButton->setEnabled(!isSearching || isPaused);
    m_cancelButton->setEnabled(isSearching);
    
    if (isPaused) {
        m_pauseButton->setText("继续");
        m_pauseButton->setEnabled(true);
        disconnect(m_pauseButton, &QPushButton::clicked, this, &SearchWidget::onPause);
        connect(m_pauseButton, &QPushButton::clicked, this, &SearchWidget::onResume);
    } else {
        m_pauseButton->setText("暂停");
        m_pauseButton->setEnabled(isSearching);
        disconnect(m_pauseButton, &QPushButton::clicked, this, &SearchWidget::onResume);
        connect(m_pauseButton, &QPushButton::clicked, this, &SearchWidget::onPause);
    }
    
    // 更新进度条状态
    if (!isSearching && !isPaused) {
        m_progressBar->setValue(0);
        m_progressInfoLabel->clear();
    }
    
    // 更新选项面板状态
    m_optionsGroupBox->setEnabled(!isSearching);
}

void SearchWidget::startSearch()
{
    // 重置状态
    resetState();
    
    // 开始搜索
    if (m_engine) {
        m_isSearching = true;
        updateUI();
        
        m_engine->startSearch(m_currentQuery);
    } else {
        QMessageBox::warning(this, "错误", "搜索引擎未初始化");
    }
}

void SearchWidget::resetState()
{
    // 重置UI状态
    m_statusLabel->setText("就绪");
    m_progressBar->setValue(0);
    m_progressInfoLabel->clear();
    
    // 清空结果
    clearResults();
    
    // 重置搜索状态
    m_isSearching = false;
}

void SearchWidget::clearResults()
{
    if (m_resultModel) {
        m_resultModel->removeRows(0, m_resultModel->rowCount());
    }
}

void SearchWidget::addResult(const std::shared_ptr<ISearchResult>& result)
{
    // 检查参数
    if (!result || !m_resultModel) {
        return;
    }
    
    auto fileResult = std::dynamic_pointer_cast<IFileResult>(result);
    if (!fileResult) {
        return;
    }
    
    // 创建模型项
    QList<QStandardItem*> items;
    
    // 名称列
    QStandardItem *nameItem = new QStandardItem();
    nameItem->setText(fileResult->title());
    nameItem->setIcon(fileResult->icon());
    nameItem->setData(fileResult->path(), Qt::UserRole);
    items.append(nameItem);
    
    // 路径列
    QStandardItem *pathItem = new QStandardItem(QFileInfo(fileResult->path()).path());
    items.append(pathItem);
    
    // 大小列
    QStandardItem *sizeItem = new QStandardItem();
    qint64 size = fileResult->size();
    if (size < 1024) {
        sizeItem->setText(QString("%1 B").arg(size));
    } else if (size < 1024 * 1024) {
        sizeItem->setText(QString("%1 KB").arg(size / 1024.0, 0, 'f', 2));
    } else if (size < 1024 * 1024 * 1024) {
        sizeItem->setText(QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2));
    } else {
        sizeItem->setText(QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2));
    }
    sizeItem->setData(size, Qt::UserRole);
    items.append(sizeItem);
    
    // 修改时间列
    QStandardItem *timeItem = new QStandardItem(fileResult->modifiedTime().toString("yyyy-MM-dd HH:mm:ss"));
    items.append(timeItem);
    
    // 类型列
    QStandardItem *typeItem = new QStandardItem();
    if (fileResult->isDir()) {
        typeItem->setText("文件夹");
    } else {
        QFileInfo fileInfo(fileResult->path());
        QString suffix = fileInfo.suffix().toLower();
        if (!suffix.isEmpty()) {
            typeItem->setText(suffix.toUpper() + " 文件");
        } else {
            typeItem->setText("文件");
        }
    }
    items.append(typeItem);
    
    // 添加到模型
    m_resultModel->appendRow(items);
    
    // 文件内容搜索结果的特殊处理
    auto contentResult = std::dynamic_pointer_cast<ContentResultItem>(result);
    if (contentResult) {
        int row = m_resultModel->rowCount() - 1;
        
        // 设置额外的显示信息
        QString contentPreview = contentResult->content();
        int lineNumber = contentResult->lineNumber();
        
        if (!contentPreview.isEmpty() && lineNumber > 0) {
            // 获取行列号
            int columnNumber = contentResult->columnNumber();
            QString lineInfo = QString("行 %1, 列 %2: ").arg(lineNumber).arg(columnNumber);
            
            // 跟随内容预览
            nameItem->setText(fileResult->title() + " - " + lineInfo + contentPreview);
            
            // 添加工具提示
            nameItem->setToolTip(contentPreview);
            
            // 添加标志，表示这是内容搜索结果
            nameItem->setData(true, Qt::UserRole + 1);
        }
    }
}

void SearchWidget::updateProgressInfo(const SearchProgress& progress)
{
    // 更新进度条
    if (progress.totalItems > 0) {
        int percent = qMin(100, static_cast<int>(progress.progressPercent));
        m_progressBar->setValue(percent);
    } else {
        // 不确定进度
        m_progressBar->setMaximum(0);
    }
    
    // 更新信息标签
    QString info = QString("已处理: %1, 已匹配: %2").arg(progress.processedItems).arg(progress.matchedItems);
    
    if (!progress.currentPath.isEmpty()) {
        info += QString(" | %1").arg(progress.currentPath);
    }
    
    m_progressInfoLabel->setText(info);
}

void SearchWidget::onResultCallback(const std::shared_ptr<ISearchResult>& result)
{
    // 这里是每当有新的搜索结果时调用的回调函数
    // 可以用于直接处理结果，而不需要等待一批结果
    // 例如，可以在这里对结果进行过滤或应用特殊处理
    
    // 当前实现只是打印调试信息
    qDebug() << "结果回调：" << result->title();
} 