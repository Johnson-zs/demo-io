#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QString>
#include <QVector>
#include "filedata.h"

// 搜索引擎接口
class ISearchEngine
{
public:
    virtual ~ISearchEngine() = default;
    
    // 更新搜索路径
    virtual void updateSearchPath(const QString &path) = 0;
    
    // 搜索文件
    virtual QVector<FileData> searchFiles(const QString &keyword) const = 0;
    
    // 获取所有文件
    virtual QVector<FileData> getAllFiles(int limit = -1) const = 0;
};

#endif // SEARCHENGINE_H 