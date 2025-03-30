#include <dfm-search/search_filter.h>

#include <QRegularExpression>
#include <QDebug>
#include <QDir>

namespace DFM {
namespace Search {

FilenameFilter::FilenameFilter(const QString& pattern, bool caseSensitive)
    : pattern_(pattern)
    , caseSensitive_(caseSensitive)
{
}

bool FilenameFilter::matches(const QFileInfo& fileInfo) const
{
    // 如果是通配符模式（包含*或?）
    if (pattern_.contains('*') || pattern_.contains('?')) {
        QRegularExpression::PatternOptions options = 
            caseSensitive_ ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
        
        // 将通配符转换为正则表达式
        QString regexPattern = QRegularExpression::escape(pattern_);
        regexPattern.replace("\\*", ".*");
        regexPattern.replace("\\?", ".");
        
        QRegularExpression regex(regexPattern, options);
        return regex.match(fileInfo.fileName()).hasMatch();
    } 
    // 精确匹配
    else {
        if (caseSensitive_) {
            return fileInfo.fileName() == pattern_;
        } else {
            return fileInfo.fileName().compare(pattern_, Qt::CaseInsensitive) == 0;
        }
    }
}

QString FilenameFilter::description() const
{
    return QString("文件名过滤器: %1 (%2)").arg(
        pattern_, 
        caseSensitive_ ? "区分大小写" : "不区分大小写"
    );
}

// FilterFactory的文件名过滤器创建方法
std::shared_ptr<FilenameFilter> FilterFactory::createFilenameFilter(
    const QString& pattern, bool caseSensitive) {
    
    return std::make_shared<FilenameFilter>(pattern, caseSensitive);
}

} // namespace Search
} // namespace DFM 