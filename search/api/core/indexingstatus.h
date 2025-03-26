namespace UniversalSearch {

class IndexingStatus {
public:
    // 索引状态
    enum class State {
        Idle,               // 空闲
        Initializing,       // 初始化中
        Indexing,           // 索引中
        Paused,             // 已暂停
        Finalizing,         // 完成中
        Error               // 错误
    };
    
    IndexingStatus();
    
    State state() const;
    void setState(State state);
    
    QString error() const;
    void setError(const QString& error);
    
    int progress() const;
    void setProgress(int progress);
    
    QDateTime lastUpdated() const;
    void setLastUpdated(const QDateTime& time);
    
    QDateTime startTime() const;
    void setStartTime(const QDateTime& time);
    
    QDateTime finishTime() const;
    void setFinishTime(const QDateTime& time);
    
    // 统计信息
    int processedItemCount() const;
    void setProcessedItemCount(int count);
    
    int totalItemCount() const;
    void setTotalItemCount(int count);
    
    qint64 processedBytes() const;
    void setProcessedBytes(qint64 bytes);
    
    // 自定义状态信息
    void addStatusInfo(const QString& key, const QVariant& value);
    QVariant statusInfo(const QString& key) const;
    QVariantMap allStatusInfo() const;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 