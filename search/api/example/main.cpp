#include <QApplication>
#include <QDebug>
#include <UniversalSearch/SearchManager>
#include <UniversalSearch/IndexManager>
#include <UniversalSearch/SearchQuery>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 获取搜索管理器实例
    auto& searchManager = UniversalSearch::SearchManager::instance();
    
    // 获取所有可用的搜索提供者
    auto providers = searchManager.availableProviders();
    qDebug() << "可用搜索提供者:";
    for (const auto& provider : providers) {
        qDebug() << "  -" << provider->name() << ":" << provider->description();
    }
    
    // 创建搜索查询
    UniversalSearch::SearchQuery query("搜索关键词");
    query.setType(UniversalSearch::SearchQuery::Type::FileName);
    query.setSearchPaths({QDir::homePath()});
    query.setLimit(20);
    
    // 连接结果信号
    QObject::connect(&searchManager, &UniversalSearch::SearchManager::resultsAvailable,
        [](const QString& sessionId, const UniversalSearch::SearchResultList& results) {
            qDebug() << "接收到" << results.size() << "个结果:";
            for (const auto& result : results) {
                qDebug() << "  -" << result.title() << ":" << result.uri();
            }
        }
    );
    
    // 连接完成信号
    QObject::connect(&searchManager, &UniversalSearch::SearchManager::searchFinished,
        [](const QString& sessionId) {
            qDebug() << "搜索完成, ID:" << sessionId;
        }
    );
    
    // 开始搜索
    QString sessionId = searchManager.startSearch(query);
    qDebug() << "开始搜索, ID:" << sessionId;
    
    return app.exec();
} 