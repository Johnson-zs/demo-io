#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QLocale>
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT TextAnalysisOptions {
public:
    void setLanguage(const QLocale& locale);
    QLocale language() const;
    
    void setEnableStemming(bool enable);
    bool isStemming() const;
    
    void setRemoveStopWords(bool remove);
    bool isRemovingStopWords() const;
    
    void setCustomStopWords(const QStringList& stopWords);
    QStringList customStopWords() const;
    
    void setTokenizeNumerals(bool tokenize);
    bool isTokenizingNumerals() const;
    
    // 拼音特性
    void setEnablePinyinAnalysis(bool enable);
    bool isPinyinAnalysisEnabled() const;
    
    // 中文分词特性
    void setChineseSegmentationMode(int mode); // 0=单字, 1=词组, 2=混合
    int chineseSegmentationMode() const;
    
private:
    QLocale m_language;
    bool m_enableStemming = true;
    bool m_removeStopWords = true;
    QStringList m_customStopWords;
    bool m_tokenizeNumerals = true;
    bool m_enablePinyin = false;
    int m_chineseSegmentationMode = 2;
};

class QTSEARCHKIT_EXPORT TextAnalyzer : public QObject {
    Q_OBJECT
public:
    explicit TextAnalyzer(QObject* parent = nullptr);
    
    // 设置配置选项
    void setOptions(const TextAnalysisOptions& options);
    TextAnalysisOptions options() const;
    
    // 分词处理
    QStringList tokenize(const QString& text) const;
    
    // 词干提取
    QString stem(const QString& word) const;
    QStringList stems(const QStringList& words) const;
    
    // 停用词过滤
    QStringList removeStopWords(const QStringList& tokens) const;
    
    // 同义词扩展
    QStringList expandSynonyms(const QString& word) const;
    
    // 用于搜索优化的文本标准化
    QString normalizeForSearch(const QString& text) const;
    
    // 拼音转换（适用于中文搜索）
    QString textToPinyin(const QString& text, bool firstLetterOnly = false) const;
    
    // 工厂方法
    static std::shared_ptr<TextAnalyzer> createForLocale(const QLocale& locale);
    
private:
    TextAnalysisOptions m_options;
};

} // namespace QtSearchKit 