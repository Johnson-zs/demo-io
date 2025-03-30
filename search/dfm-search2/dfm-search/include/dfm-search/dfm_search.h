#ifndef DFM_SEARCH_H
#define DFM_SEARCH_H

/**
 * @file dfm_search.h
 * @brief DFM-Search库主头文件
 * 
 * 这个头文件包含了DFM-Search库的所有公共接口
 */

// 核心搜索引擎
#include "search_engine.h"

// 索引引擎
#include "index_engine.h"

// 搜索过滤器
#include "search_filter.h"

// 应用程序搜索
#include "app_search.h"

// OCR搜索
#include "ocr_search.h"

// 插件系统
#include "plugin_system.h"

/**
 * @namespace DFM::Search
 * @brief DFM-Search库命名空间
 */
namespace DFM {
namespace Search {

/**
 * @brief 获取库版本号
 * @return 库版本号字符串
 */
QString version();

/**
 * @brief 初始化搜索库
 * 
 * 在使用搜索库之前必须调用此函数进行初始化
 * 
 * @param pluginDirs 插件目录列表
 * @return 是否初始化成功
 */
bool initialize(const QStringList& pluginDirs = {});

/**
 * @brief 关闭搜索库
 * 
 * 在应用程序退出前调用此函数清理资源
 */
void shutdown();

/**
 * @brief 搜索库是否已初始化
 * @return 是否已初始化
 */
bool isInitialized();

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
void setLogLevel(int level);

/**
 * @brief 设置搜索库配置
 * @param config 配置参数
 */
void setConfig(const QVariantMap& config);

/**
 * @brief 获取搜索库配置
 * @return 当前配置
 */
QVariantMap getConfig();

/**
 * @brief 便捷函数：创建文件名搜索引擎
 * @param indexed 是否使用索引
 * @return 搜索引擎实例
 */
std::shared_ptr<SearchEngine> createFilenameSearchEngine(bool indexed = false);

/**
 * @brief 便捷函数：创建内容搜索引擎
 * @param indexed 是否使用索引
 * @return 搜索引擎实例
 */
std::shared_ptr<SearchEngine> createContentSearchEngine(bool indexed = true);

/**
 * @brief 便捷函数：创建应用搜索引擎
 * @return 应用搜索引擎实例
 */
std::shared_ptr<AppSearchEngine> createAppSearchEngine();

/**
 * @brief 便捷函数：创建OCR搜索引擎
 * @return OCR搜索引擎实例
 */
std::shared_ptr<OCRSearchEngine> createOCRSearchEngine();

} // namespace Search
} // namespace DFM

#endif // DFM_SEARCH_H 