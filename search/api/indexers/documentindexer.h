namespace UniversalSearch {

class DocumentIndexer : public Indexer {
    Q_OBJECT
public:
    DocumentIndexer();
    virtual ~DocumentIndexer();
    
    // Indexer接口实现
    QString id() const override { return "document_indexer"; }
    QString name() const override { return "文档内容索引器"; }
    QString description() const override { return "为文档内容创建全文索引"; }
    
    QStringList supportedContentTypes() const override;
    QStringList supportedLocations() const override;
    
    QString startIndexing(const IndexingOptions& options) override;
    void stopIndexing(const QString& sessionId) override;
    void pauseIndexing(const QString& sessionId) override;
    void resumeIndexing(const QString& sessionId) override;
    
    void updateIndex() override;
    void resetIndex() override;
    
    IndexingStatus getStatus() const override;
    IndexingStatistics getStatistics() const override;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 