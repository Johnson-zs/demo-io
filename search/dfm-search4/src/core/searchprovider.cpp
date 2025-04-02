#include <dfm6-search/searchprovider.h>

namespace DFM6 {
namespace Search {

// 私有实现类
class SearchProviderData
{
public:
    SearchProviderData() = default;
};

SearchProvider::SearchProvider(QObject *parent)
    : QObject(parent),
      d(std::make_unique<SearchProviderData>())
{
}

SearchProvider::~SearchProvider() = default;

}  // namespace Search
}  // namespace DFM6 