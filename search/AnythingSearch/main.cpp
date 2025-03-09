#include "mainwindow.h"
#include "anythingsearcher.h"
#include "mocksearcher.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用信息
    QApplication::setApplicationName("AnythingSearch");
    QApplication::setApplicationVersion("1.0");
    
    // 命令行解析
    QCommandLineParser parser;
    parser.setApplicationDescription("文件搜索应用");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加使用Mock搜索器的选项
    QCommandLineOption mockOption(QStringList() << "m" << "mock", 
                                 "使用Mock搜索器代替DBus搜索");
    parser.addOption(mockOption);
    
    // 添加模拟延迟选项
    QCommandLineOption delayOption(QStringList() << "d" << "delay", 
                                  "设置Mock搜索器的模拟延迟(毫秒)",
                                  "delay", "100");
    parser.addOption(delayOption);
    
    // 添加模拟错误选项
    QCommandLineOption errorOption(QStringList() << "e" << "errors", 
                                  "模拟搜索错误");
    parser.addOption(errorOption);
    
    // 添加最大文件数选项
    QCommandLineOption maxFilesOption(QStringList() << "f" << "max-files", 
                                     "设置Mock搜索器返回的最大文件数",
                                     "count", "1000");
    parser.addOption(maxFilesOption);
    
    // 解析命令行参数
    parser.process(a);
    
    // 创建主窗口
    MainWindow w;
    
    // 根据命令行选项创建搜索器
    SearcherInterface *searcher = nullptr;
    
    if (parser.isSet(mockOption)) {
        // 使用Mock搜索器
        MockSearcher *mockSearcher = new MockSearcher(&w);
        
        // 设置模拟延迟
        if (parser.isSet(delayOption)) {
            mockSearcher->setSimulatedDelay(parser.value(delayOption).toInt());
        }
        
        // 设置是否模拟错误
        mockSearcher->setSimulateErrors(parser.isSet(errorOption));
        
        // 设置最大文件数
        if (parser.isSet(maxFilesOption)) {
            mockSearcher->setMaxFileCount(parser.value(maxFilesOption).toInt());
        }
        
        searcher = mockSearcher;
    } else {
        // 使用真实DBus搜索器
        searcher = new AnythingSearcher(&w);
    }
    
    // 设置搜索器
    w.setSearcher(searcher);
    
    w.show();
    return a.exec();
}
