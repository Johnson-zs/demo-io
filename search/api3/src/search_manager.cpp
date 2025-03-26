#include "qsearch/search_manager.h"
#include "qsearch/index_manager.h"
#include "qsearch/engines/engine_factory.h"
#include "qsearch/worker_manager.h"
#include "qsearch/ipc/ipc_channel.h"
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QtConcurrent>
#include <atomic>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>

namespace QSearch {

struct SearchTask {
    int id;
    SearchQuery query;
    SearchOptions options;
    SearchState state = SearchState::Idle;
    QString stateMessage;
    std::atomic<double> progress{0.0};
    SearchResult results;
    std::function<void(const ResultItem&)> resultCallback;
};

struct SearchManager::Impl {
    QMap<int, SearchTask*> activeTasks;
    QMutex tasksMutex;
    int nextSearchId = 1;
    QScopedPointer<IndexManager> indexMgr;
    QScopedPointer<IPC::IPCChannel> searchChannel;
    bool useWorkers = true;
    
    Impl() : indexMgr(new IndexManager()) {
        // 创建到搜索Worker的IPC通道
        if (WorkerManager::instance().isWorkerRunning(WorkerType::Search)) {
            searchChannel.reset(IPC::IPCChannel::create(
                IPC::IPCChannel::DBus, "org.qsearch.SearchWorker"));
            
            // 连接信号
            if (searchChannel->connect()) {
                QObject::connect(searchChannel.data(), &IPC::IPCChannel::responseReceived,
                    [this](const QString& requestId, const QJsonObject& response) {
                        handleWorkerResponse(requestId, response);
                    });
                
                QObject::connect(searchChannel.data(), &IPC::IPCChannel::notificationReceived,
                    [this](const QString& event, const QJsonObject& params) {
                        handleWorkerNotification(event, params);
                    });
            } else {
                // IPC连接失败，回退到本地模式
                useWorkers = false;
            }
        } else {
            // Worker未运行，回退到本地模式
            useWorkers = false;
        }
    }
    
    void handleWorkerResponse(const QString& requestId, const QJsonObject& response) {
        // 处理Worker的异步响应
        int searchId = response["searchId"].toInt();
        bool success = response["success"].toBool();
        
        QMutexLocker locker(&tasksMutex);
        if (activeTasks.contains(searchId)) {
            auto* task = activeTasks[searchId];
            
            if (response["type"].toString() == "searchResult") {
                // 处理搜索结果
                QJsonArray items = response["items"].toArray();
                for (const auto& item : items) {
                    ResultItem resultItem;
                    // 从JSON解析ResultItem
                    resultItem.name = item["name"].toString();
                    resultItem.path = item["path"].toString();
                    // ... 解析其他字段 ...
                    
                    task->results.addItem(resultItem);
                    if (task->resultCallback) {
                        task->resultCallback(resultItem);
                    }
                }
            } else if (response["type"].toString() == "searchComplete") {
                // 搜索完成
                task->state = success ? SearchState::Completed : SearchState::Error;
                task->stateMessage = response["message"].toString();
                task->progress = 1.0;
            }
        }
    }
    
    void handleWorkerNotification(const QString& event, const QJsonObject& params) {
        int searchId = params["searchId"].toInt();
        
        QMutexLocker locker(&tasksMutex);
        if (activeTasks.contains(searchId)) {
            auto* task = activeTasks[searchId];
            
            if (event == "progressUpdate") {
                task->progress = params["progress"].toDouble();
            } else if (event == "stateChanged") {
                task->state = static_cast<SearchState>(params["state"].toInt());
                task->stateMessage = params["message"].toString();
            } else if (event == "resultFound") {
                ResultItem item;
                // 从params解析ResultItem
                // ...
                
                if (task->resultCallback) {
                    task->resultCallback(item);
                }
            }
        }
    }
    
    ~Impl() {
        QMutexLocker locker(&tasksMutex);
        for (auto task : activeTasks.values()) {
            if (task->workerThread) {
                task->workerThread->quit();
                task->workerThread->wait();
                delete task->workerThread;
            }
            delete task;
        }
    }
    
    int getNextSearchId() {
        QMutexLocker locker(&tasksMutex);
        return nextSearchId++;
    }
    
    void cleanupTask(int searchId) {
        QMutexLocker locker(&tasksMutex);
        if (activeTasks.contains(searchId)) {
            auto task = activeTasks.take(searchId);
            if (task->workerThread) {
                task->workerThread->quit();
                task->workerThread->wait();
                delete task->workerThread;
            }
            delete task;
        }
    }
};

// 单例实现
SearchManager& SearchManager::instance() {
    static SearchManager instance;
    return instance;
}

SearchManager::SearchManager() : d(new Impl) {
}

SearchManager::~SearchManager() {
}

int SearchManager::startSearch(const SearchQuery& query, const SearchOptions& options) {
    QMutexLocker locker(&d->tasksMutex);
    
    // 创建搜索任务
    int searchId = d->nextSearchId++;
    auto* task = new SearchTask{
        .id = searchId,
        .query = query,
        .options = options,
        .state = SearchState::Idle
    };
    d->activeTasks[searchId] = task;
    
    locker.unlock();
    
    // 根据是否使用Worker模式选择执行方式
    if (d->useWorkers) {
        // 通过IPC启动远程搜索
        QJsonObject params;
        params["searchId"] = searchId;
        params["query"] = queryToJson(query);
        params["options"] = optionsToJson(options);
        
        d->searchChannel->sendRequestAsync("startSearch", params);
    } else {
        // 使用本地搜索实现
        // ... 本地搜索代码 ...
    }
    
    emit searchStarted(searchId);
    return searchId;
}

bool SearchManager::pauseSearch(int searchId) {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return false;
    }
    
    auto task = d->activeTasks[searchId];
    bool result = false;
    
    QMetaObject::invokeMethod(task->engine.data(), [task, &result]() {
        result = task->engine->pause();
    }, Qt::BlockingQueuedConnection);
    
    if (result) {
        task->state = SearchState::Paused;
        emit searchPaused(searchId);
    }
    
    return result;
}

bool SearchManager::resumeSearch(int searchId) {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return false;
    }
    
    auto task = d->activeTasks[searchId];
    if (task->state != SearchState::Paused) {
        return false;
    }
    
    bool result = false;
    QMetaObject::invokeMethod(task->engine.data(), [task, &result]() {
        result = task->engine->resume();
    }, Qt::BlockingQueuedConnection);
    
    if (result) {
        task->state = SearchState::Searching;
        emit searchResumed(searchId);
    }
    
    return result;
}

bool SearchManager::stopSearch(int searchId) {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return false;
    }
    
    auto task = d->activeTasks[searchId];
    bool result = false;
    
    QMetaObject::invokeMethod(task->engine.data(), [task, &result]() {
        result = task->engine->stop();
    }, Qt::BlockingQueuedConnection);
    
    if (result) {
        emit searchStopped(searchId);
    }
    
    return result;
}

SearchState SearchManager::searchState(int searchId) const {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return SearchState::Error;
    }
    
    return d->activeTasks[searchId]->state;
}

QString SearchManager::searchStateMessage(int searchId) const {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return "无效的搜索ID";
    }
    
    return d->activeTasks[searchId]->stateMessage;
}

double SearchManager::searchProgress(int searchId) const {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return 0.0;
    }
    
    return d->activeTasks[searchId]->progress;
}

SearchResult SearchManager::getResults(int searchId) const {
    QMutexLocker locker(&d->tasksMutex);
    if (!d->activeTasks.contains(searchId)) {
        return SearchResult();
    }
    
    return d->activeTasks[searchId]->results;
}

IndexManager* SearchManager::indexManager() const {
    return d->indexMgr.data();
}

void SearchManager::setResultCallback(int searchId, std::function<void(const ResultItem&)> callback) {
    QMutexLocker locker(&d->tasksMutex);
    if (d->activeTasks.contains(searchId)) {
        d->activeTasks[searchId]->resultCallback = callback;
    }
}

QFuture<SearchResult> SearchManager::searchAsync(const SearchQuery& query, const SearchOptions& options) {
    return QtConcurrent::run([this, query, options]() {
        return search(query, options);
    });
}

SearchResult SearchManager::search(const SearchQuery& query, const SearchOptions& options) {
    // 同步搜索方法 - 阻塞直到搜索完成
    SearchResult result;
    QEventLoop loop;
    
    int searchId = startSearch(query, options);
    if (searchId < 0) {
        return result;
    }
    
    // 设置结果回调
    setResultCallback(searchId, [&result](const ResultItem& item) {
        result.addItem(item);
    });
    
    // 监听搜索完成或错误信号
    connect(this, &SearchManager::searchCompleted, this, [&loop, searchId](int id) {
        if (id == searchId) {
            loop.quit();
        }
    });
    
    connect(this, &SearchManager::searchError, this, [&loop, searchId](int id, const QString&) {
        if (id == searchId) {
            loop.quit();
        }
    });
    
    // 等待搜索完成
    loop.exec();
    
    return result;
}

} // namespace QSearch 