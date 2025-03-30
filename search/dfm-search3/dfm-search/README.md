# DFM-Search：现代C++/Qt6搜索库

DFM-Search是一个现代化的C++/Qt6搜索库，为Linux桌面环境提供高度灵活、可扩展的搜索功能框架。

## 特性

- **多种搜索类型**：支持文件名搜索、文件内容搜索和应用程序搜索
- **灵活的搜索选项**：支持大小写敏感/不敏感、正则表达式、模糊匹配等
- **搜索机制抽象**：同时支持索引型和实时型搜索机制
- **异步搜索**：所有搜索操作在后台线程中进行，不阻塞UI
- **高度可扩展**：易于添加新的搜索类型和功能
- **现代C++**：充分利用C++17特性
- **Qt集成**：与Qt6无缝集成，支持信号槽机制

## 架构设计

DFM-Search采用模块化设计，主要包含以下组件：

1. **搜索查询(SearchQuery)**：定义搜索参数，如关键词、路径、选项等
2. **搜索结果(SearchResult)**：表示搜索结果，支持不同类型的结果
3. **搜索提供者(SearchProvider)**：提供具体搜索功能的插件系统
4. **搜索管理器(SearchManager)**：统一管理所有搜索操作的中央控制器

## 使用示例

```cpp
// 创建搜索查询
DFMSearch::SearchQuery query("example");
query.addSearchPath("/home/user");
query.addFlag(DFMSearch::SearchFlag::FuzzyMatch);

// 获取搜索管理器
auto manager = DFMSearch::SearchManager::instance();

// 注册提供者
manager->registerProvider([]() {
    return std::make_unique<DFMSearch::FileNameDataProvider>();
}, DFMSearch::SearchType::FileName);

// 设置搜索类型
manager->setSearchTypes({DFMSearch::SearchType::FileName});

// 设置查询
manager->setQuery(query);

// 处理结果
QObject::connect(manager, &DFMSearch::SearchManager::resultFound,
               [](const DFMSearch::SearchResult& result) {
    qDebug() << "找到:" << result.toString();
});

// 开始搜索
manager->start();
```

## 编译与安装

### 依赖项

- C++17兼容编译器
- CMake 3.16+
- Qt 6.0+

### 构建步骤

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## 示例应用

DFM-Search包含一个命令行示例应用程序，展示了库的基本功能：

```bash
# 基本用法
dfm-search-cli "keyword"

# 搜索文件名
dfm-search-cli -t file "example"

# 搜索文件内容
dfm-search-cli -t content "example"

# 使用正则表达式搜索
dfm-search-cli -r "\.txt$"

# 在指定路径中搜索
dfm-search-cli -p /home/user "example"
```

## 许可证

该项目采用LGPL-3.0许可证。 