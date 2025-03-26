#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <optional>
#include <functional>
#include "qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchQuery {
public:
    SearchQuery(const QString& query = QString());
    
    // 基本查询属性
    QString query() const;
    void setQuery(const QString& query);
    
    // 查询限制
    int limit() const;
    void setLimit(int limit);
    
    int offset() const;
    void setOffset(int offset);
    
    // 支持泛型灵活的查询选项配置
    template<typename T>
    void setOption(const QString& name, const T& value) {
        m_options.insert(name, QVariant::fromValue(value));
    }
    
    template<typename T>
    std::optional<T> option(const QString& name) const {
        if (!m_options.contains(name))
            return std::nullopt;
        
        QVariant value = m_options.value(name);
        if (value.canConvert<T>())
            return value.value<T>();
        
        return std::nullopt;
    }
    
    bool hasOption(const QString& name) const;
    void clearOption(const QString& name);
    void clearAllOptions();
    
    // 便捷方法 - 常用搜索选项
    void setSearchMode(SearchMode mode);
    SearchMode searchMode() const;
    
    void setCaseSensitive(bool sensitive);
    bool isCaseSensitive() const;
    
    void setWholeWord(bool wholeWord);
    bool isWholeWord() const;
    
    void setFuzzyThreshold(float threshold); // 0.0-1.0
    float fuzzyThreshold() const;
    
    void enablePinyin(bool enable);
    bool isPinyinEnabled() const;
    
    // 文件搜索特定便捷方法
    void setIncludeHidden(bool include);
    bool includeHidden() const;
    
    void setFileTypes(const QStringList& types);
    QStringList fileTypes() const;
    
    // 布尔搜索支持
    void setBooleanOperators(const QStringList& operators);
    QStringList booleanOperators() const;
    
    // 排序控制
    void setSortBy(const QString& field);
    QString sortBy() const;
    
    void setSortOrder(Qt::SortOrder order);
    Qt::SortOrder sortOrder() const;
    
private:
    QString m_query;
    int m_limit = 100;
    int m_offset = 0;
    QVariantMap m_options;
};

} // namespace QtSearchKit 