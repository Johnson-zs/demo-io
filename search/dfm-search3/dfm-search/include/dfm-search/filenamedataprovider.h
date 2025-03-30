#pragma once

#include "searchprovider.h"
#include <QThread>
#include <memory>

namespace DFMSearch {

class FileNameDataProviderPrivate;
class SearchWorker;

/**
 * @brief 文件名搜索提供者类
 * 
 * 提供基于文件名的搜索功能
 */
class DFMSEARCH_EXPORT FileNameDataProvider : public SearchProvider
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FileNameDataProvider(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FileNameDataProvider() override;

    /**
     * @brief 获取提供者名称
     * @return 名称
     */
    QString name() const override;
    
    /**
     * @brief 获取提供者描述
     * @return 描述
     */
    QString description() const override;
    
    /**
     * @brief 获取支持的搜索类型
     * @return 搜索类型
     */
    SearchType searchType() const override;
    
    /**
     * @brief 获取搜索机制
     * @return 搜索机制
     */
    SearchMechanism mechanism() const override;
    
    /**
     * @brief 停止搜索
     * @return 是否成功停止
     */
    bool stop() override;

protected:
    /**
     * @brief 执行实际搜索
     * @return 是否成功
     */
    bool doSearch() override;

private slots:
    void onWorkerFinished();
    void onWorkerResult(const SearchResult& result);
    void onWorkerProgress(int progress);
    void onWorkerError(const QString& error);

private:
    std::unique_ptr<FileNameDataProviderPrivate> d_ptr;
};

} // namespace DFMSearch 