#ifndef SEARCH_UTILITY_H
#define SEARCH_UTILITY_H

#include <QString>
#include <QStringList>
#include <QFileInfo>

namespace DFM6 {
namespace Search {
namespace Utils {

/**
 * @brief 搜索工具类
 * 
 * 提供通用的搜索辅助函数
 */
class SearchUtility
{
public:
    /**
     * @brief 检查文件是否匹配文件类型过滤器
     * 
     * @param fileInfo 文件信息
     * @param filters 文件类型过滤器
     * @return 是否匹配
     */
    static bool matchesFileType(const QFileInfo &fileInfo, const QStringList &filters);
    
    /**
     * @brief 检查文件是否为文本文件
     * 
     * @param filePath 文件路径
     * @return 是否为文本文件
     */
    static bool isTextFile(const QString &filePath);
    
    /**
     * @brief 转换文件大小为可读字符串
     * 
     * @param size 文件大小（字节）
     * @return 格式化的大小字符串
     */
    static QString formatFileSize(qint64 size);
    
    /**
     * @brief 高亮文本中的关键词
     * 
     * @param text 原文本
     * @param keywords 要高亮的关键词
     * @param preTag 高亮前缀标签
     * @param postTag 高亮后缀标签
     * @return 高亮处理后的文本
     */
    static QString highlightKeywords(const QString &text, 
                                    const QStringList &keywords,
                                    const QString &preTag = "<b>",
                                    const QString &postTag = "</b>");
};

}  // namespace Utils
}  // namespace Search
}  // namespace DFM6

#endif // SEARCH_UTILITY_H 