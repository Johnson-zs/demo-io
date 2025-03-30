#include "searchwidget.h"
#include "ui_searchwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItem>
#include <QProcess>
#include <QDateTime>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWidget),
    m_resultModel(new QStandardItemModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_searchType(DFM::Search::SearchType::FileName),
    m_searchMode(DFM::Search::SearchMode::Realtime),
    m_isSearching(false)
{
    ui->setupUi(this);
    
    // 设置模型列
    m_resultModel->setColumnCount(4);
    m_resultModel->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("路径") << tr("大小") << tr("修改时间"));
    
    // 设置代理模型
    m_proxyModel->setSourceModel(m_resultModel);
    ui->resultView->setModel(m_proxyModel);
    
    // 调整列宽
    ui->resultView->setColumnWidth(0, 200);  // 名称
    ui->resultView->setColumnWidth(1, 300);  // 路径
    ui->resultView->setColumnWidth(2, 100);  // 大小
    ui->resultView->setColumnWidth(3, 150);  // 修改时间
    
    // 连接信号和槽
    connect(ui->searchButton, &QPushButton::clicked, this, &SearchWidget::onSearch);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SearchWidget::onCancel);
    connect(ui->pauseButton, &QPushButton::clicked, this, [this]() {
        if (m_searchEngine->state() == DFM::Search::SearchState::Searching) {
            onPause();
        } else if (m_searchEngine->state() == DFM::Search::SearchState::Paused) {
            onResume();
        }
    });
    connect(ui->clearButton, &QPushButton::clicked, this, &SearchWidget::onClear);
    connect(ui->browseButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择搜索目录"), 
                                                       QDir::homePath());
        if (!dir.isEmpty()) {
            ui->pathEdit->setText(dir);
        }
    });
    
    // 连接搜索框回车事件
    connect(ui->searchEdit, &QLineEdit::returnPressed, this, &SearchWidget::onSearch);
    
    // 连接结果双击事件
    connect(ui->resultView, &QTreeView::doubleClicked, this, &SearchWidget::onResultItemActivated);
    
    // 连接重建索引按钮
    connect(ui->rebuildIndexButton, &QPushButton::clicked, this, [this]() {
        if (m_searchEngine && m_searchMode == DFM::Search::SearchMode::Indexed) {
            auto indexedEngine = std::dynamic_pointer_cast<DFM::Search::IndexedFilenameSearchEngine>(m_searchEngine);
            if (indexedEngine) {
                indexedEngine->rebuildIndex();
                ui->statusLabel->setText(tr("正在重建索引..."));
                ui->rebuildIndexButton->setEnabled(false);
            }
        }
    });
    
    // 连接搜索选项变更事件
    QList<QCheckBox*> checkBoxes = {
        ui->caseSensitiveCheck, ui->regexCheck, ui->fuzzyMatchCheck, 
        ui->wholeWordCheck, ui->recursiveCheck, ui->hiddenFilesCheck,
        ui->followSymlinksCheck
    };
    
    for (QCheckBox* checkBox : checkBoxes) {
        connect(checkBox, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    }
    
    // 初始化搜索引擎
    resetSearchEngine();
    
    // 初始化 UI
    updateUi();
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::setSearchType(DFM::Search::SearchType type)
{
    m_searchType = type;
    resetSearchEngine();
    
    // 更新索引重建按钮可见性
    if (m_searchType == DFM::Search::SearchType::FileName && 
        m_searchMode == DFM::Search::SearchMode::Indexed) {
        ui->rebuildIndexButton->setVisible(true);
    } else {
        ui->rebuildIndexButton->setVisible(false);
    }
}

void SearchWidget::setSearchMode(DFM::Search::SearchMode mode)
{
    if (m_searchMode != mode) {
        m_searchMode = mode;
        resetSearchEngine();
        updateUi();
    }
}

void SearchWidget::onSearch()
{
    if (m_isSearching) {
        return;
    }
    
    QString query = ui->searchEdit->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::warning(this, tr("搜索错误"), tr("请输入搜索关键词"));
        return;
    }
    
    m_currentQuery = query;
    clearResults();
    
    // 设置搜索范围
    DFM::Search::SearchScope scope;
    
    // 添加搜索路径
    QString path = ui->pathEdit->text();
    if (path.startsWith("~/")) {
        path.replace(0, 1, QDir::homePath());
    }
    scope.includePaths << path;
    
    // 添加排除模式
    QString excludePatterns = ui->excludeEdit->text();
    if (!excludePatterns.isEmpty()) {
        scope.excludePatterns = excludePatterns.split(",", Qt::SkipEmptyParts);
    }
    
    m_searchEngine->setSearchScope(scope);
    
    // 设置搜索选项
    DFM::Search::SearchOptions options = getSearchOptions();
    
    m_searchEngine->setSearchOptions(options);
    
    // 开始搜索
    if (m_searchEngine->prepare()) {
        if (m_searchEngine->startSearch(query)) {
            m_isSearching = true;
            ui->statusLabel->setText(tr("正在搜索: %1").arg(query));
            ui->progressBar->setRange(0, 0);  // 显示未确定进度条
            updateUi();
        } else {
            ui->statusLabel->setText(tr("无法开始搜索"));
        }
    } else {
        ui->statusLabel->setText(tr("搜索引擎准备失败"));
    }
}

void SearchWidget::onCancel()
{
    if (!m_isSearching) {
        return;
    }
    
    if (m_searchEngine) {
        m_searchEngine->cancelSearch();
        ui->statusLabel->setText(tr("搜索已取消"));
        m_isSearching = false;
        updateUi();
    }
}

void SearchWidget::onPause()
{
    if (!m_isSearching) {
        return;
    }
    
    if (m_searchEngine) {
        m_searchEngine->pauseSearch();
        ui->statusLabel->setText(tr("搜索已暂停"));
        ui->pauseButton->setText(tr("继续"));
        updateUi();
    }
}

void SearchWidget::onResume()
{
    if (!m_isSearching) {
        return;
    }
    
    if (m_searchEngine && m_searchEngine->state() == DFM::Search::SearchState::Paused) {
        m_searchEngine->resumeSearch();
        ui->statusLabel->setText(tr("正在搜索: %1").arg(m_currentQuery));
        ui->pauseButton->setText(tr("暂停"));
        updateUi();
    }
}

void SearchWidget::onClear()
{
    ui->searchEdit->clear();
    clearResults();
    ui->statusLabel->setText(tr("就绪"));
    ui->progressBar->setValue(0);
    updateUi();
}

void SearchWidget::onOptionsChanged()
{
    // 检查排他性选项
    if (ui->wholeWordCheck->isChecked() && ui->fuzzyMatchCheck->isChecked()) {
        QCheckBox *sender = qobject_cast<QCheckBox*>(QObject::sender());
        if (sender == ui->wholeWordCheck) {
            ui->fuzzyMatchCheck->setChecked(false);
        } else {
            ui->wholeWordCheck->setChecked(false);
        }
    }
    
    // 如果正在搜索，使用新选项更新搜索
    if (m_isSearching && m_searchEngine) {
        m_searchEngine->setOptions(getSearchOptions());
    }
}

void SearchWidget::onResultsReady(const DFM::Search::SearchResultSet& results)
{
    for (auto it = results.begin(); it != results.end(); ++it) {
        addResult(*it);
    }
    
    ui->statusLabel->setText(tr("已找到 %1 个结果").arg(m_resultModel->rowCount()));
}

void SearchWidget::onProgressChanged(const DFM::Search::SearchProgress& progress)
{
    updateProgressInfo(progress);
}

void SearchWidget::onStateChanged(DFM::Search::SearchState state)
{
    switch (state) {
        case DFM::Search::SearchState::Idle:
            ui->statusLabel->setText(tr("就绪"));
            m_isSearching = false;
            break;
        case DFM::Search::SearchState::Preparing:
            ui->statusLabel->setText(tr("准备中..."));
            break;
        case DFM::Search::SearchState::Searching:
            ui->statusLabel->setText(tr("正在搜索: %1").arg(m_currentQuery));
            m_isSearching = true;
            break;
        case DFM::Search::SearchState::Paused:
            ui->statusLabel->setText(tr("搜索已暂停"));
            break;
        case DFM::Search::SearchState::Completed:
            ui->statusLabel->setText(tr("搜索完成，找到 %1 个结果").arg(m_resultModel->rowCount()));
            m_isSearching = false;
            break;
        case DFM::Search::SearchState::Cancelled:
            ui->statusLabel->setText(tr("搜索已取消"));
            m_isSearching = false;
            break;
        case DFM::Search::SearchState::Error:
            ui->statusLabel->setText(tr("搜索出错"));
            m_isSearching = false;
            break;
    }
    
    updateUi();
}

void SearchWidget::onSearchCompleted(bool success)
{
    m_isSearching = false;
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(100);
    
    if (success) {
        ui->statusLabel->setText(tr("搜索完成，找到 %1 个结果").arg(m_resultModel->rowCount()));
    } else {
        ui->statusLabel->setText(tr("搜索未完成"));
    }
    
    updateUi();
}

void SearchWidget::onErrorOccurred(const QString& error)
{
    ui->statusLabel->setText(tr("错误: %1").arg(error));
    m_isSearching = false;
    updateUi();
}

void SearchWidget::onResultItemActivated(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    
    // 获取源模型中的实际索引
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (!sourceIndex.isValid()) {
        return;
    }
    
    // 获取路径列的值 (第2列)
    QModelIndex pathIndex = m_resultModel->index(sourceIndex.row(), 1);
    QString path = m_resultModel->data(pathIndex).toString();
    
    // 尝试打开文件或目录
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    } else {
        QMessageBox::warning(this, tr("打开失败"), tr("文件不存在: %1").arg(path));
    }
}

void SearchWidget::setupUi()
{
    // 设置进度条初始状态
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    
    // 隐藏或显示索引相关控件
    bool showIndexControls = (m_searchMode == DFM::Search::SearchMode::Indexed);
    ui->rebuildIndexButton->setVisible(showIndexControls);
}

void SearchWidget::updateUi()
{
    // 更新按钮状态
    ui->searchButton->setEnabled(!m_isSearching);
    ui->cancelButton->setEnabled(m_isSearching);
    ui->pauseButton->setEnabled(m_isSearching);
    
    // 调整暂停按钮文本
    if (m_searchEngine && m_searchEngine->state() == DFM::Search::SearchState::Paused) {
        ui->pauseButton->setText(tr("继续"));
    } else {
        ui->pauseButton->setText(tr("暂停"));
    }
    
    // 更新索引按钮状态
    bool showIndexControls = (m_searchMode == DFM::Search::SearchMode::Indexed);
    ui->rebuildIndexButton->setVisible(showIndexControls);
    
    // 如果是索引搜索引擎，更新索引状态
    if (m_searchMode == DFM::Search::SearchMode::Indexed && m_searchEngine) {
        auto indexedEngine = std::dynamic_pointer_cast<DFM::Search::IndexedFilenameSearchEngine>(m_searchEngine);
        if (indexedEngine) {
            ui->rebuildIndexButton->setEnabled(indexedEngine->state() != DFM::Search::SearchState::Preparing);
        }
    }
}

void SearchWidget::setupSearchOptions()
{
    // 根据搜索类型和模式启用或禁用某些选项
    ui->fuzzyMatchCheck->setEnabled(m_searchType == DFM::Search::SearchType::FileName);
    
    // 索引搜索特定选项
    if (m_searchMode == DFM::Search::SearchMode::Indexed) {
        ui->rebuildIndexButton->setVisible(true);
    } else {
        ui->rebuildIndexButton->setVisible(false);
    }
    
    // 连接搜索选项变更信号
    connect(ui->caseSensitiveCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->regexCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->fuzzyMatchCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->wholeWordCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->recursiveCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->hiddenFilesCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    connect(ui->followSymlinksCheck, &QCheckBox::toggled, this, &SearchWidget::onOptionsChanged);
    
    // 设置排他性选项
    connect(ui->fuzzyMatchCheck, &QCheckBox::toggled, [this](bool checked) {
        if (checked && ui->wholeWordCheck->isChecked()) {
            ui->wholeWordCheck->setChecked(false);
        }
    });
    
    connect(ui->wholeWordCheck, &QCheckBox::toggled, [this](bool checked) {
        if (checked && ui->fuzzyMatchCheck->isChecked()) {
            ui->fuzzyMatchCheck->setChecked(false);
        }
    });
}

void SearchWidget::setupResultsView()
{
    // 设置列宽
    ui->resultView->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->resultView->header()->setStretchLastSection(true);
}

void SearchWidget::clearResults()
{
    m_resultModel->removeRows(0, m_resultModel->rowCount());
}

void SearchWidget::addResult(const DFM::Search::SearchResult& result)
{
    if (!result) {
        return;
    }
    
    QList<QStandardItem*> rowItems;
    
    // 名称列
    QStandardItem* nameItem = new QStandardItem();
    nameItem->setText(result->title());
    nameItem->setIcon(result->icon());
    rowItems.append(nameItem);
    
    // 路径列
    QStandardItem* pathItem = new QStandardItem();
    if (result->type() == DFM::Search::SearchResultItem::ResultType::File || 
        result->type() == DFM::Search::SearchResultItem::ResultType::Content) {
        auto fileResult = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(result);
        if (fileResult) {
            pathItem->setText(fileResult->path());
        } else {
            pathItem->setText(result->description());
        }
    } else {
        pathItem->setText(result->description());
    }
    rowItems.append(pathItem);
    
    // 大小列
    QStandardItem* sizeItem = new QStandardItem();
    if (result->type() == DFM::Search::SearchResultItem::ResultType::File) {
        auto fileResult = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(result);
        if (fileResult) {
            // 格式化文件大小
            qint64 size = fileResult->size();
            if (size < 1024) {
                sizeItem->setText(QString("%1 B").arg(size));
            } else if (size < 1024 * 1024) {
                sizeItem->setText(QString("%1 KB").arg(size / 1024.0, 0, 'f', 1));
            } else if (size < 1024 * 1024 * 1024) {
                sizeItem->setText(QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1));
            } else {
                sizeItem->setText(QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1));
            }
            sizeItem->setData(size, Qt::UserRole);
        }
    } else {
        sizeItem->setText("-");
    }
    rowItems.append(sizeItem);
    
    // 修改时间列
    QStandardItem* timeItem = new QStandardItem();
    if (result->type() == DFM::Search::SearchResultItem::ResultType::File) {
        auto fileResult = std::dynamic_pointer_cast<DFM::Search::FileResultItem>(result);
        if (fileResult && fileResult->modifiedTime().isValid()) {
            timeItem->setText(fileResult->modifiedTime().toString("yyyy-MM-dd HH:mm:ss"));
            timeItem->setData(fileResult->modifiedTime(), Qt::UserRole);
        }
    }
    rowItems.append(timeItem);
    
    // 添加结果行
    m_resultModel->appendRow(rowItems);
}

void SearchWidget::updateProgressInfo(const DFM::Search::SearchProgress& progress)
{
    if (progress.totalItems > 0) {
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(qRound(progress.progressPercent));
    } else {
        ui->progressBar->setRange(0, 0);  // 不确定进度
    }
    
    QString statusText;
    
    if (progress.currentPath.isEmpty()) {
        statusText = tr("已处理: %1 个项目, 匹配: %2 个")
                      .arg(progress.processedItems)
                      .arg(progress.matchedItems);
    } else {
        statusText = tr("已处理: %1 个项目, 匹配: %2 个, 当前: %3")
                      .arg(progress.processedItems)
                      .arg(progress.matchedItems)
                      .arg(progress.currentPath);
    }
    
    ui->statusLabel->setText(statusText);
}

void SearchWidget::resetSearchEngine()
{
    // 断开之前的连接
    if (m_searchEngine) {
        disconnect(m_searchEngine.get(), nullptr, this, nullptr);
    }
    
    // 创建新的搜索引擎
    m_searchEngine = DFM::Search::SearchEngineManager::instance()->createEngine(m_searchType, m_searchMode);
    
    if (!m_searchEngine) {
        // 如果请求的引擎不可用，则使用默认的文件名搜索
        m_searchType = DFM::Search::SearchType::FileName;
        m_searchMode = DFM::Search::SearchMode::Realtime;
        m_searchEngine = DFM::Search::SearchEngineManager::instance()->createEngine(m_searchType, m_searchMode);
    }
    
    if (m_searchEngine) {
        // 连接信号与槽
        connect(m_searchEngine.get(), &DFM::Search::ISearchEngine::resultsReady,
                this, &SearchWidget::onResultsReady);
        connect(m_searchEngine.get(), &DFM::Search::ISearchEngine::progressChanged,
                this, &SearchWidget::onProgressChanged);
        connect(m_searchEngine.get(), &DFM::Search::ISearchEngine::stateChanged,
                this, &SearchWidget::onStateChanged);
        connect(m_searchEngine.get(), &DFM::Search::ISearchEngine::searchCompleted,
                this, &SearchWidget::onSearchCompleted);
        connect(m_searchEngine.get(), &DFM::Search::ISearchEngine::errorOccurred,
                this, &SearchWidget::onErrorOccurred);
    }
    
    // 更新 UI 以匹配新引擎
    setupUi();
    setupSearchOptions();
    m_isSearching = false;
}

DFM::Search::SearchOptions SearchWidget::getSearchOptions() const
{
    DFM::Search::SearchOptions options = DFM::Search::SearchOptionFlag::None;
    
    if (ui->caseSensitiveCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::CaseSensitive;
    }
    
    if (ui->regexCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::RegexPattern;
    }
    
    if (ui->fuzzyMatchCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::FuzzyMatch;
    }
    
    if (ui->wholeWordCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::WholeWord;
    }
    
    if (ui->recursiveCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::RecursiveSearch;
    }
    
    if (ui->hiddenFilesCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::HiddenFiles;
    }
    
    if (ui->followSymlinksCheck->isChecked()) {
        options |= DFM::Search::SearchOptionFlag::FollowSymlinks;
    }
    
    return options;
}