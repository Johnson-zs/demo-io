FileNameSearchResult::FileNameSearchResult()
    : SearchResult()
{
    // 替换为FileNameSearchResultData
    d.reset(new FileNameSearchResultData());
}

FileNameSearchResult::FileNameSearchResult(const QString &path)
    : SearchResult()
{
    d.reset(new FileNameSearchResultData(path));
}

// 辅助转换函数
FileNameSearchResultData* FileNameSearchResult::data()
{
    return static_cast<FileNameSearchResultData*>(d.get());
}

const FileNameSearchResultData* FileNameSearchResult::data() const
{
    return static_cast<const FileNameSearchResultData*>(d.get());
}

// 其他方法实现
QString FileNameSearchResult::matchType() const
{
    return data()->matchType;
}

void FileNameSearchResult::setMatchType(const QString &type)
{
    data()->matchType = type;
} 