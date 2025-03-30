#pragma once

#include <QString>
#include <QVariant>
#include <QDateTime>

namespace Framework {

/**
 * @brief 框架版本信息
 */
struct Version {
    static const int Major = 1;
    static const int Minor = 0;
    static const int Patch = 0;
    
    static QString toString() {
        return QString("%1.%2.%3").arg(Major).arg(Minor).arg(Patch);
    }
};

/**
 * @brief 系统常量定义
 */
struct Constants {
    // 通信相关
    static const int DEFAULT_PORT = 50000;
    static const int HEARTBEAT_INTERVAL = 5000; // 毫秒
    static const int CONNECTION_TIMEOUT = 30000; // 毫秒
    
    // 资源相关
    static const int MAX_TASKS_PER_WORKER = 10;
};

/**
 * @brief 工具函数
 */
namespace Utils {
    /**
     * @brief 生成唯一ID
     * @return 唯一ID字符串
     */
    QString generateUniqueId();
    
    /**
     * @brief 获取当前时间戳
     * @return 当前时间戳
     */
    QDateTime now();
    
    /**
     * @brief 获取系统CPU使用率
     * @return CPU使用率（0-100）
     */
    double getCpuUsage();
    
    /**
     * @brief 获取系统可用内存
     * @return 可用内存（字节）
     */
    qint64 getAvailableMemory();
    
    /**
     * @brief 获取系统总内存
     * @return 总内存（字节）
     */
    qint64 getTotalMemory();
}

} // namespace Framework 