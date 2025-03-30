#pragma once

#include "searchprovider.h"
#include <memory>

namespace DFMSearch {

class AppDataProviderPrivate;

/**
 * @brief 应用程序搜索提供者类
 * 
 * 提供对系统已安装应用程序的搜索功能
 */
class DFMSEARCH_EXPORT AppDataProvider : public SearchProvider
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AppDataProvider(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~AppDataProvider() override;

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
     * @brief 设置应用程序目录
     * @param directories 应用程序目录列表
     */
    void setAppDirectories(const QStringList& directories);
    
    /**
     * @brief 获取应用程序目录
     * @return 应用程序目录列表
     */
    QStringList appDirectories() const;

protected:
    /**
     * @brief 执行实际搜索
     * @return 是否成功
     */
    bool doSearch() override;

private:
    std::unique_ptr<AppDataProviderPrivate> d_ptr;
};

} // namespace DFMSearch 