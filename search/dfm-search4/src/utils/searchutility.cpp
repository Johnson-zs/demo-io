#include "searchutility.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace DFM6 {
namespace Search {
namespace Utils {

bool SearchUtility::matchesFileType(const QFileInfo &fileInfo, const QStringList &filters)
{
    if (filters.isEmpty()) {
        return true;  // 没有过滤器则匹配所有
    }
    
    QString suffix = fileInfo.suffix().toLower();
    for (const QString &filter : filters) {
        if (filter.startsWith("*.")) {
            // 扩展名过滤
            QString filterSuffix = filter.mid(2).toLower();
            if (suffix == filterSuffix) {
                return true;
            }
        } else if (filter.contains('/')) {
            // MIME 类型过滤
            // 实际实现需要检查文件的 MIME 类型
            // 这里简化处理
            if (filter.endsWith("/*")) {
                QString mimeGroup = filter.left(filter.length() - 2);
                // 检查 MIME 类型组是否匹配
                // ...
            } else {
                // 检查完整 MIME 类型是否匹配
                // ...
            }
        }
    }
    
    return false;
}

bool SearchUtility::isTextFile(const QString &filePath)
{
    // 简单实现：检查文件前4KB是否包含空字符
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    const int checkSize = 4096;
    QByteArray data = file.read(checkSize);
    file.close();
    
    // 如果文件小于4KB且包含空字符，或前4KB包含空字符，视为二进制文件
    return !data.contains('\0');
}

QString SearchUtility::formatFileSize(qint64 size)
{
    constexpr qint64 KB = 1024;
    constexpr qint64 MB = 1024 * KB;
    constexpr qint64 GB = 1024 * MB;
    constexpr qint64 TB = 1024 * GB;
    
    if (size < KB) {
        return QString("%1 B").arg(size);
    } else if (size < MB) {
        return QString("%1 KB").arg(size / double(KB), 0, 'f', 1);
    } else if (size < GB) {
        return QString("%1 MB").arg(size / double(MB), 0, 'f', 1);
    } else if (size < TB) {
        return QString("%1 GB").arg(size / double(GB), 0, 'f', 1);
    } else {
        return QString("%1 TB").arg(size / double(TB), 0, 'f', 1);
    }
}

QString SearchUtility::highlightKeywords(const QString &text, 
                                       const QStringList &keywords,
                                       const QString &preTag,
                                       const QString &postTag)
{
    if (keywords.isEmpty() || text.isEmpty()) {
        return text;
    }
    
    QString result = text;
    
    for (const QString &keyword : keywords) {
        if (keyword.isEmpty()) {
            continue;
        }
        
        // 使用正则表达式替换关键词（区分大小写）
        QRegularExpression regex(QRegularExpression::escape(keyword),
                               QRegularExpression::CaseInsensitiveOption);
        result.replace(regex, preTag + "\\0" + postTag);
    }
    
    return result;
}

}  // namespace Utils
}  // namespace Search
}  // namespace DFM6 