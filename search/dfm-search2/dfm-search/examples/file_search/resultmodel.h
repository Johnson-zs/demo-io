#ifndef RESULTMODEL_H
#define RESULTMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include <vector>

#include <dfm-search/search_engine.h>

/**
 * @brief 搜索结果模型类
 * 
 * 用于在TreeView中显示搜索结果
 */
class ResultModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    explicit ResultModel(QObject *parent = nullptr);
    ~ResultModel() override;
    
    // QAbstractItemModel接口实现
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // 设置搜索结果
    void setResults(const DFM::Search::SearchResult &results);
    
    // 清空结果
    void clear();
    
    // 获取结果项
    std::shared_ptr<DFM::Search::SearchResultItem> getResultItem(const QModelIndex &index) const;
    
private:
    // 存储搜索结果
    DFM::Search::SearchResult results_;
    
    // 列名
    QStringList headerLabels_;
};

#endif // RESULTMODEL_H 