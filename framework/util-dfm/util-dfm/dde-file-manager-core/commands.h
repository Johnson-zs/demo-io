#ifndef DFM_COMMANDS_H
#define DFM_COMMANDS_H

namespace DFM {

// 主程序向Worker发送的命令
enum Command {
    // 基础命令
    CMD_NONE = 0,
    CMD_HOST = 1,
    CMD_CONNECT = 2,
    CMD_DISCONNECT = 3,
    CMD_CONFIG = 4,
    
    // 任务命令
    CMD_GET = 100,
    CMD_PUT = 101,
    CMD_STAT = 102,
    CMD_LISTDIR = 103,
    CMD_MKDIR = 104,
    CMD_RENAME = 105,
    CMD_COPY = 106,
    CMD_DEL = 107,
    CMD_CHMOD = 108,
    CMD_SPECIAL = 109,
    CMD_TRUNCATE = 110,
    
    // DU特定命令
    CMD_DU = 200,
    CMD_DU_RECURSIVE = 201,
    
    // Worker响应
    CMD_DATA = 500,
    CMD_ERROR = 501,
    CMD_FINISHED = 502,
    CMD_STAT_ENTRY = 503,
    CMD_LIST_ENTRIES = 504,
    
    // 特殊事件
    CMD_MESSAGEBOXANSWER = 600,
    CMD_RESUMEANSWER = 601,
    CMD_HOST_INFO = 602
};

// 错误码
enum Error {
    ERR_NONE = 0,
    ERR_CANNOT_CONNECT = 1,
    ERR_CANNOT_AUTHENTICATE = 2,
    ERR_WORKER_DIED = 3,
    ERR_CANNOT_ENTER_DIRECTORY = 4,
    ERR_ACCESS_DENIED = 5,
    ERR_UNKNOWN = 6,
    ERR_WORKER_TIMEOUT = 7,
    ERR_UNSUPPORTED_ACTION = 8,
    ERR_DISK_FULL = 9,
    ERR_FILE_ALREADY_EXIST = 10
};

// 消息类型
enum MessageType {
    MSG_INFORMATION = 1,
    MSG_WARNING = 2,
    MSG_ERROR = 3,
    MSG_QUESTION = 4
};

// 任务标志
enum JobFlag {
    JOB_DEFAULT = 0,
    JOB_OVERWRITE = 1,
    JOB_RESUME = 2,
    JOB_RECURSIVE = 4
};

// 工作模式
enum WorkerMode {
    MODE_FILE = 0,
    MODE_DIR = 1,
    MODE_LINK = 2
};

}  // namespace DFM

#endif // DFM_COMMANDS_H 