namespace UniversalSearch {

class FileNameSearchProvider : public SearchProvider {
    Q_OBJECT
public:
    FileNameSearchProvider();
    virtual ~FileNameSearchProvider();
    
    // SearchProvider接口实现
    QString id() const override { return "file_name"; }
    QString name() const override { return "文件名搜索"; }
    QString description() const override { return "基于文件名的快速搜索"; }
    QIcon icon() const override;
    
    QStringList supportedQueryTypes() const override;
    bool requiresIndexing() const override { return true; }
    QString associatedIndexerId() const override { return "file_system_indexer"; }
    
    bool canHandleQuery(const SearchQuery& query) const override;
    QString executeQuery(const SearchQuery& query) override;
    void cancelQuery(const QString& queryId) override;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 