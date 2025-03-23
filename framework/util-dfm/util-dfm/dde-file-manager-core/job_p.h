#ifndef DFM_JOB_P_H
#define DFM_JOB_P_H

#include "job.h"

namespace DFM {

/**
 * @class JobPrivate
 * @brief Job类的私有实现
 */
class JobPrivate {
public:
    /**
     * @brief 构造函数
     * @param qq 指向公共类的指针
     */
    JobPrivate(Job *qq)
        : q(qq)
        , error(0)
        , running(false)
        , suspended(false)
        , jobFlags(0)
    {}
    
    /**
     * @brief 析构函数
     */
    virtual ~JobPrivate() {}
    
    /**
     * @brief 文件操作类型枚举
     */
    enum FileOperationType {
        Other,      ///< 其他操作
        Get,        ///< 获取文件
        Put,        ///< 上传文件
        List,       ///< 列目录
        Mkdir,      ///< 创建目录
        Rename,     ///< 重命名
        Copy,       ///< 复制
        Delete,     ///< 删除
        Stat,       ///< 获取状态
        Special,    ///< 特殊操作
        DiskUsage   ///< 磁盘使用统计
    };

    Job *q;                    ///< 指向公共类的指针
    int error;                 ///< 错误代码
    QString errorText;         ///< 错误文本
    bool running;              ///< 是否正在运行
    bool suspended;            ///< 是否已挂起
    JobFlags jobFlags;         ///< 任务标志
    FileOperationType operationType; ///< 操作类型
};

} // namespace DFM

#endif // DFM_JOB_P_H 