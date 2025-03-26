#include <QCoreApplication>
#include <QDebug>
#include <qsearch/search_manager.h>
#include <qsearch/search_query.h>
#include <qsearch/search_result.h>

using namespace QSearch;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 获取搜索管理器实例
    SearchManager& searchManager = SearchManager::instance();
    
    // 创建索引(可选)
    auto* indexManager = searchManager.indexManager();
    indexManager->initIndex(IndexType::Filename, {QDir::homePath()});
    
    // 连接信号
    QObject::connect(&searchManager, &SearchManager::resultItemFound, 
                     [](int searchId, const ResultItem& item) {
        qDebug() << "找到结果:" << item.path;
    });
    
    QObject::connect(&searchManager, &SearchManager::searchCompleted,
                     [&searchManager](int searchId) {
        qDebug() << "搜索完成，共找到" 
                 << searchManager.getResults(searchId).count() 
                 << "个结果";
        QCoreApplication::quit();
    });
    
    // 创建搜索查询
    SearchQuery query;
    query.setType(QueryType::Filename)
         .setMatchType(MatchType::Contains)
         .setText("document")
         .setPaths({QDir::homePath()});
    
    // 开始搜索
    int searchId = searchManager.startSearch(query);
    qDebug() << "开始搜索，搜索ID:" << searchId;
    
    return app.exec();
} 