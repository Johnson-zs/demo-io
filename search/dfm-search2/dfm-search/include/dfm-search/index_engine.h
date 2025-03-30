#ifndef DFM_INDEX_ENGINE_H
#define DFM_INDEX_ENGINE_H

#include <memory>
#include <vector>
#include <functional>
#include <chrono>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QUrl>

namespace DFM {
namespace Search {

/**
 * @brief 索引状态枚举
 */
enum class IndexStatus {
    Uninitialized,    ///< 未初始化
    Initializing,     ///< 初始化中
    Ready,            ///< 就绪
    Updating,         ///< 更新中
    Error             ///< 错误状态
};

/**
 * @brief 索引类型枚举
 */
enum class IndexType {
    Filename,         ///< 文件名索引
    FileContent,      ///< 文件内容索引
    FileMetadata,     ///< 文件元数据索引
    Application,      ///< 应用程序索引
    Custom            ///< 自定义索引
};

/**
 * @brief 索引配置类
 */
class IndexConfig {
public:
    QStringList indexPaths;                      ///< 索引路径
    QStringList excludePaths;                    ///< 排除路径
    QStringList filePatterns;                    ///< 文件模式
    bool followSymlinks{false};                  ///< 是否跟踪符号链接
    std::chrono::minutes updateInterval{60};     ///< 自动更新间隔
    QString indexStoragePath;                    ///< 索引存储路径
    int maxCacheSize{100};                       ///< 最大缓存大小(MB)
    bool enableWatcher{true};                    ///< 启用文件系统监视器
};

/**
 * @brief 索引引擎接口类
 * 
 * 负责创建和维护搜索索引
 */
class IndexEngine : public QObject {
    Q_OBJECT
    
public:
    // 使用PIMPL模式
    class Impl;
    
    IndexEngine();
    virtual ~IndexEngine();
    
    // 索引操作
    bool initialize(const IndexConfig& config);
    bool update(const QStringList& paths = {});
    bool rebuild();
    bool clear();
    bool isInitialized() const;
    
    // 状态信息
    IndexStatus status() const;
    int progressPercentage() const;
    QDateTime lastUpdateTime() const;
    qint64 indexSize() const;
    int itemCount() const;
    
    // 回调设置
    using ProgressCallback = std::function<void(int percentage)>;
    using StatusCallback = std::function<void(IndexStatus)>;
    
    void setProgressCallback(ProgressCallback callback);
    void setStatusCallback(StatusCallback callback);
    
signals:
    void progressChanged(int percentage);
    void statusChanged(DFM::Search::IndexStatus status);
    void indexReady();
    void indexError(const QString& errorMessage);
    
private:
    std::unique_ptr<Impl> d;
};

/**
 * @brief 索引引擎工厂
 * 
 * 用于创建不同类型的索引引擎实例
 */
class IndexEngineFactory {
public:
    static IndexEngineFactory& instance();
    
    // 创建索引引擎
    std::shared_ptr<IndexEngine> createEngine(IndexType type);
    
    // 注册自定义索引引擎工厂
    using EngineCreator = std::function<std::shared_ptr<IndexEngine>()>;
    void registerEngineCreator(IndexType type, EngineCreator creator);
    
private:
    IndexEngineFactory() = default;
    ~IndexEngineFactory() = default;
    
    IndexEngineFactory(const IndexEngineFactory&) = delete;
    IndexEngineFactory& operator=(const IndexEngineFactory&) = delete;
};

} // namespace Search
} // namespace DFM

#endif // DFM_INDEX_ENGINE_H 