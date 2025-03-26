基于Modern C++和Qt6的Linux桌面级搜索/索引库设计文档
1. 总体架构
本设计提出一个名为 UniversalSearch 的库，旨在为Linux桌面环境提供统一、灵活且可扩展的搜索接口。通过抽象化搜索逻辑和索引管理，使应用程序能够轻松集成各类搜索功能，而无需关心底层实现细节。
1.1 核心设计理念
抽象与解耦: 将搜索功能与索引管理解耦，实现面向接口编程
可扩展性: 采用插件架构，支持动态添加新的搜索提供者和索引管理器
异步操作: 所有耗时操作均为异步，避免阻塞用户界面
统一接口: 为不同类型的搜索提供统一的接口

@startuml

package "UniversalSearch库" {
  [SearchManager] as SM
  [IndexManager] as IM
  [QueryProcessor] as QP
  [ProviderRegistry] as PR
}

package "搜索提供者" {
  [FileNameSearchProvider]
  [FullTextSearchProvider]
  [ImageSearchProvider]
  [ApplicationSearchProvider]
}

package "索引管理器" {
  [FileSystemIndexer]
  [DocumentIndexer]
  [ImageIndexer]
}

package "应用程序" {
  [QtApplication]
}

cloud "后端服务" {
  [DBusServices]
  [ExternalIndexers]
}

QtApplication --> SM: 使用
SM --> PR: 注册/获取提供者
SM --> QP: 处理查询
SM --> IM: 管理索引

PR --> FileNameSearchProvider: 管理
PR --> FullTextSearchProvider: 管理
PR --> ImageSearchProvider: 管理
PR --> ApplicationSearchProvider: 管理

IM --> FileSystemIndexer: 管理
IM --> DocumentIndexer: 管理
IM --> ImageIndexer: 管理

FileNameSearchProvider --> FileSystemIndexer: 使用
FullTextSearchProvider --> DocumentIndexer: 使用
ImageSearchProvider --> ImageIndexer: 使用

FileSystemIndexer --> DBusServices: 可能使用
DocumentIndexer --> ExternalIndexers: 可能使用

@enduml

