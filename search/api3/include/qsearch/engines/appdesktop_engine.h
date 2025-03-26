#pragma once

#include "search_engine.h"
#include "../global.h"

namespace QSearch {

class QSEARCH_EXPORT AppDesktopSearchEngine : public SearchEngine {
    Q_OBJECT
public:
    explicit AppDesktopSearchEngine(QObject* parent = nullptr);
    ~AppDesktopSearchEngine() override;
    
    // 实现基类接口
    QString engineId() const override;
    QString engineName() const override;
    QString engineDescription() const override;
    
    bool supportsIndexing() const override;
    bool supportsQueryType(QueryType type) const override;
    bool supportsMatchType(MatchType type) const override;
    bool supportsFeature(const QString& feature) const override;
    
    bool prepare(const SearchQuery& query, const SearchOptions& options) override;
    bool start() override;
    bool pause() override;
    bool resume() override;
    bool stop() override;
    
    SearchState state() const override;
    double progress() const override;
    SearchResult currentResults() const override;
    
private:
    struct Impl;
    QScopedPointer<Impl> d;
};

// 定义应用元数据结构
struct AppInfo {
    QString desktopFile;      // .desktop文件路径
    QString name;             // 应用名称
    QString genericName;      // 通用名称
    QString comment;          // 应用描述
    QString exec;             // 执行命令
    QString icon;             // 图标路径
    QStringList categories;   // 应用类别
    QStringList keywords;     // 关键词
    bool hidden;              // 是否隐藏
    bool noDisplay;           // 是否不显示在菜单中
    
    // 用于排序和评分
    double relevanceScore;
};

} // namespace QSearch 