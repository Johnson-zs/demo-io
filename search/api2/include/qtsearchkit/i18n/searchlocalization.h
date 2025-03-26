#pragma once

#include <QObject>
#include <QString>
#include <QLocale>
#include <QMap>
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchLocalization : public QObject {
    Q_OBJECT
public:
    explicit SearchLocalization(QObject* parent = nullptr);
    
    // 设置语言
    void setLocale(const QLocale& locale);
    QLocale locale() const;
    
    // 支持的语言
    QList<QLocale> supportedLocales() const;
    bool isLocaleSupported(const QLocale& locale) const;
    
    // 对搜索词进行本地化处理
    QString localizeQuery(const QString& query) const;
    
    // 获取用于该语言的停用词
    QStringList stopWordsForLocale(const QLocale& locale) const;
    
    // 添加自定义停用词
    void addCustomStopWords(const QLocale& locale, const QStringList& words);
    
    // 多语言同义词管理
    void addSynonyms(const QLocale& locale, const QString& word, const QStringList& synonyms);
    QStringList synonyms(const QLocale& locale, const QString& word) const;
    
private:
    QLocale m_locale;
    QMap<QLocale, QStringList> m_stopWords;
    QMap<QLocale, QMap<QString, QStringList>> m_synonyms;
};

} // namespace QtSearchKit 