#pragma once

#include <QObject>
#include <QString>
#include <QTextDocument>
#include "searchquery.h"
#include "searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 搜索结果高亮配置
class QTSEARCHKIT_EXPORT HighlightOptions {
public:
    HighlightOptions();
    
    void setHighlightPrefix(const QString& prefix);
    QString highlightPrefix() const;
    
    void setHighlightSuffix(const QString& suffix);
    QString highlightSuffix() const;
    
    void setMaxContextLength(int length);
    int maxContextLength() const;
    
    void setContextWordsBefore(int words);
    int contextWordsBefore() const;
    
    void setContextWordsAfter(int words);
    int contextWordsAfter() const;
    
    void setMaxHighlightsPerResult(int max);
    int maxHighlightsPerResult() const;
    
private:
    QString m_prefix = "<b>";
    QString m_suffix = "</b>";
    int m_maxContextLength = 150;
    int m_contextWordsBefore = 5;
    int m_contextWordsAfter = 10;
    int m_maxHighlightsPerResult = 3;
};

// 高亮和摘要生成器
class QTSEARCHKIT_EXPORT SearchHighlighter : public QObject {
    Q_OBJECT
public:
    explicit SearchHighlighter(QObject* parent = nullptr);
    
    // 配置高亮选项
    void setOptions(const HighlightOptions& options);
    HighlightOptions options() const;
    
    // 生成高亮内容
    QString highlightText(const QString& text, const SearchQuery& query) const;
    
    // 生成摘要上下文（带高亮）
    QString generateSummary(const QString& fullText, const SearchQuery& query) const;
    
    // 处理整个结果集
    void processResults(std::shared_ptr<SearchResultSet> results, const SearchQuery& query);
    
private:
    HighlightOptions m_options;
};

} // namespace QtSearchKit 