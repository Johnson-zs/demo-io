namespace UniversalSearch {

class FullTextSearchProvider : public SearchProvider {
    Q_OBJECT
public:
    FullTextSearchProvider();
    virtual ~FullTextSearchProvider();
    
    // SearchProvider接口实现
    QString id() const override { return "full_text"; }
    QString name() const override { return "全文搜索"; }
    QString description() const override { return "文档内容的全文检索"; }
    QIcon icon() const override;
    
    QStringList supportedQueryTypes() const override;
    bool requiresIndexing() const override { return true; }
    QString associatedIndexerId() const override { return "document_indexer"; }
    
    bool canHandleQuery(const SearchQuery& query) const override;
    QString executeQuery(const SearchQuery& query) override;
    void cancelQuery(const QString& queryId) override;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 