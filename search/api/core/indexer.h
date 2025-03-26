namespace UniversalSearch {

class Indexer : public QObject {
    Q_OBJECT
public:
    virtual ~Indexer() = default;
    
    // 索引器信息
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    
    // 索引能力
    virtual QStringList supportedContentTypes() const = 0;
    virtual QStringList supportedLocations() const = 0;
    
    // 索引操作
    virtual QString startIndexing(const IndexingOptions& options) = 0;
    virtual void stopIndexing(const QString& sessionId) = 0;
    virtual void pauseIndexing(const QString& sessionId) = 0;
    virtual void resumeIndexing(const QString& sessionId) = 0;
    
    // 索引维护
    virtual void updateIndex() = 0;
    virtual void resetIndex() = 0;
    
    // 状态查询
    virtual IndexingStatus getStatus() const = 0;
    virtual IndexingStatistics getStatistics() const = 0;
    
signals:
    // 状态信号
    void indexingStarted(const QString& sessionId);
    void indexingFinished(const QString& sessionId);
    void indexingProgress(const QString& sessionId, int progress);
    void indexingError(const QString& sessionId, const QString& error);
    void statusChanged(const IndexingStatus& status);
};

using IndexerPtr = std::shared_ptr<Indexer>;

} // namespace UniversalSearch 