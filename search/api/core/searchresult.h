namespace UniversalSearch {

class SearchResult {
public:
    // 结果类型
    enum class Type {
        File,               // 文件结果
        Document,           // 文档结果
        Email,              // 邮件结果
        Image,              // 图片结果
        Application,        // 应用程序结果
        Contact,            // 联系人结果
        HelpTopic,          // 帮助主题
        Custom              // 自定义结果
    };
    
    // 构造函数
    SearchResult();
    SearchResult(Type type, const QString& uri);
    
    // 基本属性
    Type type() const;
    void setType(Type type);
    
    QString uri() const;
    void setUri(const QString& uri);
    
    QString title() const;
    void setTitle(const QString& title);
    
    QString description() const;
    void setDescription(const QString& description);
    
    QDateTime modificationTime() const;
    void setModificationTime(const QDateTime& time);
    
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    
    // 相关性评分
    double relevance() const;
    void setRelevance(double score);
    
    // 匹配项
    struct Match {
        QString field;      // 匹配的字段
        int startPos;       // 起始位置
        int length;         // 长度
        QString context;    // 上下文内容（用于预览）
    };
    
    void addMatch(const Match& match);
    QList<Match> matches() const;
    
    // 元数据
    void addMetadata(const QString& key, const QVariant& value);
    QVariant metadata(const QString& key) const;
    QVariantMap allMetadata() const;
    
    // 自定义内容
    void setCustomData(const QVariantMap& data);
    QVariantMap customData() const;
    
    // 快捷动作
    struct Action {
        QString id;
        QString name;
        QIcon icon;
        std::function<void()> callback;
    };
    
    void addAction(const Action& action);
    QList<Action> actions() const;
    
private:
    // 实现细节...
};

using SearchResultList = QList<SearchResult>;

} // namespace UniversalSearch 