#pragma once

#include "searchprovider.h"
#include <memory>

namespace DFMSearch {

class FileContentDataProviderPrivate;

/**
 * @brief 文件内容搜索提供者类
 * 
 * 提供基于文件内容的搜索功能
 */
class DFMSEARCH_EXPORT FileContentDataProvider : public SearchProvider
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FileContentDataProvider(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FileContentDataProvider() override;

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
    
    /**
     * @brief 设置文件类型过滤器
     * @param filters 文件类型过滤器（如 "*.txt *.md"）
     */
    void setFileFilters(const QStringList& filters);
    
    /**
     * @brief 获取文件类型过滤器
     * @return 文件类型过滤器
     */
    QStringList fileFilters() const;

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
    std::unique_ptr<FileContentDataProviderPrivate> d_ptr;
};

} // namespace DFMSearch 