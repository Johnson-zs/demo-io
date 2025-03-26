#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QMap>
#include "search_engine.h"
#include "../global.h"

namespace QSearch {

class QSEARCH_EXPORT EngineFactory : public QObject {
    Q_OBJECT
public:
    static EngineFactory& instance();
    
    // 获取所有可用的搜索引擎
    QStringList availableEngines() const;
    
    // 创建指定类型的搜索引擎
    QSharedPointer<SearchEngine> createEngine(const QString& engineId);
    
    // 根据查询类型创建合适的搜索引擎
    QSharedPointer<SearchEngine> createEngineForQuery(const SearchQuery& query);
    
    // 注册自定义搜索引擎
    bool registerEngine(const QString& engineId, 
                        std::function<SearchEngine*(QObject*)> factory);
    
    // 注销搜索引擎
    bool unregisterEngine(const QString& engineId);
    
private:
    // 私有构造函数(单例模式)
    EngineFactory(QObject* parent = nullptr);
    ~EngineFactory();
    
    struct Impl;
    QScopedPointer<Impl> d;
};

} // namespace QSearch 