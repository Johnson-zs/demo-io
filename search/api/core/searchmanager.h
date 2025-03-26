namespace UniversalSearch {

class SearchManager : public QObject {
    Q_OBJECT
public:
    static SearchManager& instance();
    
    // 开始搜索，返回搜索会话ID
    QString startSearch(const SearchQuery& query);
    
    // 取消搜索
    void cancelSearch(const QString& sessionId);
    
    // 暂停/恢复搜索
    void pauseSearch(const QString& sessionId);
    void resumeSearch(const QString& sessionId);
    
    // 注册搜索提供者
    void registerProvider(SearchProviderPtr provider);
    
    // 获取所有可用搜索提供者
    QList<SearchProviderPtr> availableProviders() const;
    
    // 启用/禁用搜索提供者
    void enableProvider(const QString& providerId, bool enable = true);
    
signals:
    // 搜索结果信号
    void resultsAvailable(const QString& sessionId, const SearchResultList& results);
    
    // 搜索状态变化信号
    void searchStarted(const QString& sessionId);
    void searchFinished(const QString& sessionId);
    void searchError(const QString& sessionId, const QString& errorMessage);
    void searchProgress(const QString& sessionId, int progress);
    
private:
    SearchManager();
    ~SearchManager();
    
    // 实现细节...
};

} // namespace UniversalSearch 