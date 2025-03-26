#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFuture>
#include "searchquery.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SuggestionOptions {
public:
    SuggestionOptions();
    
    void setMaxSuggestions(int max);
    int maxSuggestions() const;
    
    void setMinQueryLength(int length);
    int minQueryLength() const;
    
    void setUseFuzzyMatching(bool use);
    bool useFuzzyMatching() const;
    
    void setIncludeHistory(bool include);
    bool includeHistory() const;
    
private:
    int m_maxSuggestions = 10;
    int m_minQueryLength = 2;
    bool m_useFuzzyMatching = true;
    bool m_includeHistory = true;
};

class QTSEARCHKIT_EXPORT SearchSuggester : public QObject {
    Q_OBJECT
public:
    explicit SearchSuggester(QObject* parent = nullptr);
    
    // 配置
    void setOptions(const SuggestionOptions& options);
    SuggestionOptions options() const;
    
    // 自动完成/查询建议
    QFuture<QStringList> getSuggestions(const QString& partialQuery, SearchType type = SearchType::FileName);
    
    // 主动学习常用查询
    void learnFromQuery(const QString& query, int resultCount);
    
    // 建议来源管理
    void addCustomSuggestions(const QStringList& suggestions);
    void clearCustomSuggestions();
    
signals:
    void suggestionsReady(const QStringList& suggestions);
    
private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace QtSearchKit 