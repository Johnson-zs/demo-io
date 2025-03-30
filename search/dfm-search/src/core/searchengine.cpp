#include "dfm-search/searchengine.h"

namespace DFM {
namespace Search {
ISearchEngine::ISearchEngine(QObject *parent)
    : QObject(parent)
{

}

// 添加一个空的实现，以确保符号会被导出
ISearchEngine::~ISearchEngine() = default;

} // namespace Search
} // namespace DFM


