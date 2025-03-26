namespace UniversalSearch {

class SearchQuery {
public:
    // 构造函数
    SearchQuery();
    explicit SearchQuery(const QString& searchText);
    
    // 查询基本属性
    void setText(const QString& text);
    QString text() const;
    
    // 查询类型
    enum class Type {
        All,                // 所有类型
        FileName,           // 文件名搜索
        FullText,           // 全文搜索
        Image,              // 图片搜索
        Application,        // 应用搜索
        Contact,            // 联系人搜索
        Help,               // 帮助内容搜索
        Custom              // 自定义搜索
    };
    
    void setType(Type type);
    Type type() const;
    
    // 自定义类型
    void setCustomType(const QString& customType);
    QString customType() const;
    
    // 过滤器
    void addFilter(const QString& key, const QVariant& value);
    void removeFilter(const QString& key);
    QVariantMap filters() const;
    
    // 搜索范围
    void setSearchPaths(const QStringList& paths);
    QStringList searchPaths() const;
    
    // 排序和分页
    void setSortField(const QString& field);
    QString sortField() const;
    
    void setSortOrder(Qt::SortOrder order);
    Qt::SortOrder sortOrder() const;
    
    void setLimit(int limit);
    int limit() const;
    
    void setOffset(int offset);
    int offset() const;
    
    // 高级选项
    void setOption(const QString& key, const QVariant& value);
    QVariant option(const QString& key) const;
    QVariantMap options() const;
    
private:
    // 实现细节...
};

} // namespace UniversalSearch 