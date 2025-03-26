#pragma once

#include <QString>
#include <QStringList>
#include <QSet>
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

class QTSEARCHKIT_EXPORT SearchFeatures {
public:
    SearchFeatures();
    
    // 设置支持的搜索类型
    void addSupportedType(SearchType type);
    void removeSupportedType(SearchType type);
    bool supportsType(SearchType type) const;
    QList<SearchType> supportedTypes() const;
    
    // 设置支持的搜索模式
    void addSupportedMode(SearchMode mode);
    void removeSupportedMode(SearchMode mode);
    bool supportsMode(SearchMode mode) const;
    QList<SearchMode> supportedModes() const;
    
    // 设置支持的选项
    void addSupportedOption(const QString& option);
    void removeSupportedOption(const QString& option);
    bool supportsOption(const QString& option) const;
    QStringList supportedOptions() const;
    
    // 设置支持的排序字段
    void addSupportedSortField(const QString& field);
    bool supportsSortField(const QString& field) const;
    QStringList supportedSortFields() const;
    
    // 合并特性
    SearchFeatures& merge(const SearchFeatures& other);
    
private:
    QSet<SearchType> m_supportedTypes;
    QSet<SearchMode> m_supportedModes;
    QSet<QString> m_supportedOptions;
    QSet<QString> m_supportedSortFields;
};

} // namespace QtSearchKit 