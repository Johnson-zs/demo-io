#ifndef SEARCH_RESULT_DATA_H
#define SEARCH_RESULT_DATA_H

#include <QString>
#include <QDateTime>
#include <QVariantMap>

namespace DFM6 {
namespace Search {

/**
 * @brief SearchResult的私有实现类
 */
class SearchResultData
{
public:
    SearchResultData();
    SearchResultData(const QString &path);
    SearchResultData(const SearchResultData &other);
    
    // 公共数据字段
    QString path;
    QDateTime modifiedTime;
    qint64 size;
    QString highlightedContent;
    float score;
    bool isDirectory;
    QVariantMap customAttributes;
};

}  // namespace Search
}  // namespace DFM6

#endif // SEARCH_RESULT_DATA_H 