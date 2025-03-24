#ifndef DFM_COMMANDS_H
#define DFM_COMMANDS_H

#include "errorcodes.h"  // 包含错误代码定义

namespace DFM {

// 命令类型定义
enum CommandType {
    // 基本命令
    CMD_NONE = 0,
    CMD_GET = 10,
    CMD_PUT = 11,
    CMD_STAT = 12,
    CMD_LISTDIR = 13,
    CMD_MKDIR = 14,
    CMD_CHMOD = 15,
    CMD_COPY = 16,
    CMD_DEL = 17,
    CMD_RENAME = 18,
    CMD_SYMLINK = 19,
    CMD_SPECIAL = 20,
    
    // 文件读写命令
    CMD_OPEN = 30,
    CMD_READ = 31,
    CMD_WRITE = 32,
    CMD_SEEK = 33,
    CMD_CLOSE = 34,
    CMD_TRUNCATE = 35,
    
    // 任务控制命令
    CMD_SUSPEND = 40,
    CMD_RESUME = 41,
    CMD_CONNECTED = 42,
    CMD_ERROR = 43,
    CMD_FINISHED = 44,
    CMD_PROGRESS = 45,
    CMD_CANCELED = 46,
    
    // 元数据命令
    CMD_META_DATA = 50,
    
    // 特殊命令
    CMD_MIMETYPE = 60,
    CMD_RESOLVE = 61,
    CMD_REPARSECONFIGURATION = 62,
    CMD_HOST_INFO = 63,
    CMD_WORKER_STATUS = 64,
    CMD_DU = 65,
    CMD_DU_RECURSIVE = 66,
    
    // 用户交互命令
    CMD_MESSAGEBOX = 70,
    CMD_MESSAGEBOXANSWER = 71,
    
    // 其他命令
    CMD_SLAVE_STATUS = 80,
    CMD_SLAVE_CONNECT = 81,
    CMD_SLAVE_HOLD = 82,
    CMD_MULTI_GET = 83
};

// 消息类型
enum MessageType {
    MSG_INFORMATION = 1,
    MSG_WARNING = 2,
    MSG_ERROR = 3,
    MSG_QUESTION = 4
};

// 工作模式
enum WorkerMode {
    MODE_FILE = 0,
    MODE_DIR = 1,
    MODE_LINK = 2
};

// 其他命令相关定义可以根据需要添加

} // namespace DFM

#endif // DFM_COMMANDS_H 