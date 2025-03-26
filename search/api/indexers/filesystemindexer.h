namespace UniversalSearch {

class FileSystemIndexer : public Indexer {
    Q_OBJECT
public:
    FileSystemIndexer();
    virtual ~FileSystemIndexer();
    
    // Indexer接口实现
    QString id() const override { return "file_system_indexer"; }
    QString name() const override { return "文件系统索引器"; }
    QString description() const override { return "为文件名和基本属性创建索引"; }
    
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