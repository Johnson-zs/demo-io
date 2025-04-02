#include "filenamesearchoptions.h"

namespace DFM6 {
namespace Search {

FileNameSearchOptions::FileNameSearchOptions()
    : SearchOptions()
{
    // 设置文件名搜索的特定默认选项
    setCustomOption("pinyinEnabled", false);
    setCustomOption("fuzzySearch", false);
}

FileNameSearchOptions::FileNameSearchOptions(const FileNameSearchOptions &other)
    : SearchOptions(other)
{
    // 自定义选项已通过基类复制
}

FileNameSearchOptions::FileNameSearchOptions(FileNameSearchOptions &&other) noexcept
    : SearchOptions(std::move(other))
{
    // 自定义选项已通过基类移动
}

FileNameSearchOptions::FileNameSearchOptions(const SearchOptions &options)
    : SearchOptions(options)
{
    // 如果基类选项没有设置特定选项，设置默认值
    if (!hasCustomOption("pinyinEnabled")) {
        setCustomOption("pinyinEnabled", false);
    }
    if (!hasCustomOption("fuzzySearch")) {
        setCustomOption("fuzzySearch", false);
    }
}

FileNameSearchOptions::~FileNameSearchOptions() = default;

FileNameSearchOptions& FileNameSearchOptions::operator=(const FileNameSearchOptions &other)
{
    if (this != &other) {
        SearchOptions::operator=(other);
    }
    return *this;
}

FileNameSearchOptions& FileNameSearchOptions::operator=(FileNameSearchOptions &&other) noexcept
{
    if (this != &other) {
        SearchOptions::operator=(std::move(other));
    }
    return *this;
}

void FileNameSearchOptions::setPinyinEnabled(bool enabled)
{
    setCustomOption("pinyinEnabled", enabled);
}

bool FileNameSearchOptions::pinyinEnabled() const
{
    return customOption("pinyinEnabled").toBool();
}

void FileNameSearchOptions::setFuzzySearch(bool enabled)
{
    setCustomOption("fuzzySearch", enabled);
}

bool FileNameSearchOptions::fuzzySearch() const
{
    return customOption("fuzzySearch").toBool();
}

void FileNameSearchOptions::setFileTypes(const QStringList &types)
{
    setCustomOption("fileTypes", types);
}

QStringList FileNameSearchOptions::fileTypes() const
{
    return customOption("fileTypes").toStringList();
}

std::unique_ptr<SearchOptions> FileNameSearchOptions::clone() const
{
    return std::make_unique<FileNameSearchOptions>(*this);
}

}  // namespace Search
}  // namespace DFM6 