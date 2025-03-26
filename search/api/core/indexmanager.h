namespace UniversalSearch {

class IndexManager : public QObject {
    Q_OBJECT
public:
    static IndexManager& instance();
    
    // 获取所有索引器
    QList<IndexerPtr> availableIndexers() const;
    
    // 注册索引器
    void registerIndexer(IndexerPtr indexer);
    
    // 索引管理操作
    QString startIndexing(const QString& indexerId, const IndexingOptions& options = {});
    void stopIndexing(const QString& sessionId);
    void pauseIndexing(const QString& sessionId);
    void resumeIndexing(const QString& sessionId);
    
    // 索引状态查询
    IndexingStatus getIndexingStatus(const QString& indexerId) const;
    
    // 索引维护
    void updateIndex(const QString& indexerId);
    void resetIndex(const QString& indexerId);
    
signals:
    // 索引状态信号
    void indexingStarted(const QString& sessionId, const QString& indexerId);
    void indexingFinished(const QString& sessionId, const QString& indexerId);
    void indexingProgress(const QString& sessionId, const QString& indexerId, int progress);
    void indexingError(const QString& sessionId, const QString& indexerId, const QString& error);
    
private:
    IndexManager();
    ~IndexManager();
    
    // 实现细节...
};

} // namespace UniversalSearch 