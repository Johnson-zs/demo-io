#include <QCoreApplication>
#include <QLibrary>
#include <QPluginLoader>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <clocale>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <signal.h>
#endif

/**
 * @brief Worker加载器主函数
 * 
 * 这个程序负责加载Worker插件并运行。
 * 命令行参数格式：dde-file-manager-worker <worker-lib> <protocol> <app-socket>
 * 
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 退出代码
 */
int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "Usage: dde-file-manager-worker <worker-lib> <protocol> <app-socket>\n\n"
                        "This program is part of DDE File Manager.\n");
        return 1;
    }

    setlocale(LC_ALL, "");
    QString libname = QFile::decodeName(argv[1]);

    if (libname.isEmpty()) {
        fprintf(stderr, "Library path is empty.\n");
        return 1;
    }

    // 使用QPluginLoader定位库
    QString libpath = QPluginLoader(libname).fileName();
    if (libpath.isEmpty()) {
        fprintf(stderr, "Could not locate %s, check QT_PLUGIN_PATH\n", qPrintable(libname));
        return 1;
    }

    // 加载库
    QLibrary lib(libpath);
    if (!lib.load()) {
        fprintf(stderr, "Could not open %s: %s\n", qPrintable(libname), qPrintable(lib.errorString()));
        return 1;
    }

    // 解析主入口点
    QFunctionPointer sym = lib.resolve("kdemain");
    if (!sym) {
        fprintf(stderr, "Could not find kdemain: %s\n", qPrintable(lib.errorString()));
        return 1;
    }

    // 调试支持
    const QByteArray workerDebugWait = qgetenv("DFM_WORKER_DEBUG_WAIT");

#ifdef Q_OS_UNIX
    // 在调试模式下挂起进程，等待调试器连接
    if (workerDebugWait == "all" || workerDebugWait == argv[2]) {
        const pid_t pid = getpid();
        fprintf(stderr,
                "DFM worker: Suspending process to debug worker(s): %s\n"
                "DFM worker: 'gdb dde-file-manager-worker %d' to debug\n"
                "DFM worker: 'kill -SIGCONT %d' to continue\n",
                workerDebugWait.constData(),
                pid,
                pid);

        kill(pid, SIGSTOP);
    }
#endif

    // 调用Worker的主入口点
    int (*func)(int, char *[]) = (int (*)(int, char *[]))sym;

    // 调整传递给Worker的参数
    // 我们需要跳过worker-lib参数，让命令行更简洁
    const int newArgc = argc - 1;
    QVarLengthArray<char *, 4> newArgv(newArgc);
    newArgv[0] = argv[0];
    for (int i = 1; i < newArgc; ++i) {
        newArgv[i] = argv[i + 1];
    }

    // 启动Worker
    return func(newArgc, newArgv.data());
} 