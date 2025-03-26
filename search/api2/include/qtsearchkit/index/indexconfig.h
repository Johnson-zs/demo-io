#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT IndexConfig {
public:
    IndexConfig();
    
    // 索引名称和类型
    QString name() const;
    void setName(const QString& name);
    
    SearchType type() const;
    void setType(SearchType type);
    
    // 索引位置
    QString indexPath() const;
    void setIndexPath(const QString& path);
    
    // 要索引的来源
    QStringList sources() const;
    void setSources(const QStringList& sources);
    void addSource(const QString& source);
    void removeSource(const QString& source);
    
    // 索引排除项
    QStringList excludePatterns() const;
    void setExcludePatterns(const QStringList& patterns);
    void addExcludePattern(const QString& pattern);
    
    // 高级配置选项
    template<typename T>
    void setOption(const QString& key, const T& value) {
        m_options.insert(key, QVariant::fromValue(value));
    }
    
    QVariant option(const QString& key, const QVariant& defaultValue = QVariant()) const;
    bool hasOption(const QString& key) const;
    
    // 序列化支持
    QVariantMap toVariantMap() const;
    static IndexConfig fromVariantMap(const QVariantMap& map);
    
private:
    QString m_name;
    SearchType m_type = SearchType::FileName;
    QString m_indexPath;
    QStringList m_sources;
    QStringList m_excludePatterns;
    QVariantMap m_options;
};

} // namespace QtSearchKit 