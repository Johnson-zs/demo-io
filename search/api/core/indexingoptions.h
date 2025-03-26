namespace UniversalSearch {

class IndexingOptions {
public:
    IndexingOptions();
    
    // 索引路径
    void addPath(const QString& path);
    void removePath(const QString& path);
    QStringList paths() const;
    
    // 内容类型过滤
    void addContentType(const QString& mimeType);
    void removeContentType(const QString& mimeType);
    QStringList contentTypes() const;
    
    // 排除模式
    void addExcludePattern(const QString& pattern);
    void removeExcludePattern(const QString& pattern);
    QStringList excludePatterns() const;
    
    // 高级选项
    void setRecursive(bool recursive);
    bool isRecursive() const;
    
    void setFollowSymlinks(bool follow);
    bool followSymlinks() const;
    
    void setMaxDepth(int depth);
    int maxDepth() const;
    
    // 性能选项
    void setThreadCount(int count);
    int threadCount() const;
    
    void setBatchSize(int size);
    int batchSize() const;
    
    // 自定义选项
    void setOption(const QString& key, const QVariant& value);
    QVariant option(const QString& key) const;
    QVariantMap options() const;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 