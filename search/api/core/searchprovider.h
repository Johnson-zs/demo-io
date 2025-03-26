namespace UniversalSearch {

class SearchProvider : public QObject {
    Q_OBJECT
public:
    virtual ~SearchProvider() = default;
    
    // 提供者信息
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QIcon icon() const = 0;
    
    // 搜索能力
    virtual QStringList supportedQueryTypes() const = 0;
    virtual bool requiresIndexing() const = 0;
    virtual QString associatedIndexerId() const = 0;
    
    // 搜索操作
    virtual bool canHandleQuery(const SearchQuery& query) const = 0;
    virtual QString executeQuery(const SearchQuery& query) = 0;
    virtual void cancelQuery(const QString& queryId) = 0;
    
signals:
    // 结果和状态信号
    void resultsReady(const QString& queryId, const SearchResultList& results);
    void queryFinished(const QString& queryId);
    void queryError(const QString& queryId, const QString& errorMessage);
    void queryProgress(const QString& queryId, int progress);
};

using SearchProviderPtr = std::shared_ptr<SearchProvider>;

} // namespace UniversalSearch 