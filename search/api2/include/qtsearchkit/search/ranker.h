#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include "searchquery.h"
#include "searchresult.h"
#include "../qtsearchkit_global.h"

namespace QtSearchKit {

// 搜索结果排名因子
class QTSEARCHKIT_EXPORT RankingFactor {
public:
    enum Type {
        TextMatch,        // 文本匹配度
        Recency,          // 最近程度
        Popularity,       // 流行度
        FileType,         // 文件类型
        Size,             // 文件大小
        Path,             // 路径匹配
        Custom            // 自定义因子
    };
    
    RankingFactor(Type type = TextMatch, float weight = 1.0f);
    
    Type type() const;
    void setType(Type type);
    
    float weight() const;
    void setWeight(float weight);
    
    QString customName() const;
    void setCustomName(const QString& name);
    
private:
    Type m_type;
    float m_weight;
    QString m_customName;
};

// 自定义排名算法
class QTSEARCHKIT_EXPORT Ranker : public QObject {
    Q_OBJECT
public:
    explicit Ranker(QObject* parent = nullptr);
    
    // 设置排名因子
    void addFactor(const RankingFactor& factor);
    void removeFactor(RankingFactor::Type type);
    void setFactorWeight(RankingFactor::Type type, float weight);
    QList<RankingFactor> factors() const;
    
    // 自定义评分函数
    using ScoreFunction = std::function<float(const SearchResultItem&, const SearchQuery&)>;
    void setCustomScorer(const QString& name, ScoreFunction function);
    
    // 对结果进行排序
    void rankResults(std::shared_ptr<SearchResultSet> results, const SearchQuery& query);
    
    // 获取单个结果的得分
    float scoreItem(const SearchResultItem& item, const SearchQuery& query) const;
    
private:
    QList<RankingFactor> m_factors;
    QMap<QString, ScoreFunction> m_customScorers;
};

} // namespace QtSearchKit 