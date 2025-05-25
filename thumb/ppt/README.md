# PPT缩略图查看器 v2.0

基于Qt6开发的PowerPoint文件缩略图生成和查看工具，支持多种生成方式。

## 新功能特性 🆕

### 双重缩略图生成方式

**1. KDE风格提取（推荐）**
- 🚀 **极速生成**：直接从PPT文件内部提取已有缩略图
- 📁 **支持格式**：PPTX、DOCX、XLSX（Office Open XML）、ODP、ODT、ODS（OpenDocument）
- ⚡ **零依赖**：无需LibreOffice，使用系统unzip命令
- 🎯 **高质量**：提取原始内嵌缩略图，保持最佳质量

**2. LibreOffice转换（兼容性）**
- 🔄 **通用转换**：支持所有PPT格式，包括老版本.ppt文件
- 🛠️ **依赖要求**：需要安装LibreOffice
- 📊 **完整支持**：可处理复杂格式和特殊内容

**3. 自动检测（默认）**
- 🧠 **智能选择**：优先使用KDE方式，失败时自动切换到LibreOffice
- ⚖️ **最佳平衡**：兼顾速度和兼容性

## 技术实现

### KDE风格提取原理
模仿KDE Dolphin文件管理器的缩略图生成机制：

1. **Office Open XML文件**（.pptx/.docx/.xlsx）
   - 解析`_rels/.rels`关系文件
   - 查找缩略图关系：`http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail`
   - 提取`docProps/thumbnail.*`或媒体文件

2. **OpenDocument文件**（.odp/.odt/.ods）
   - 直接提取`Thumbnails/thumbnail.png`标准路径

3. **ZIP解压处理**
   - 使用系统`unzip`命令解压文件
   - 临时目录处理，自动清理
   - 支持多种图像格式（PNG、JPG、JPEG）

## 安装和使用

### 系统要求
- Linux系统（已测试Ubuntu/KDE）
- Qt6开发库
- unzip命令（通常系统自带）
- LibreOffice（可选，用于兼容性模式）

### 编译安装
```bash
# 克隆项目
git clone <repository-url>
cd ppt-thumbnail-viewer

# 使用qmake编译
qmake6 PPTThumbnailViewer.pro
make

# 运行程序
./build/PPTThumbnailViewer
```

### 使用方法
1. 启动程序
2. 在工具栏选择生成方式：
   - **自动检测**（推荐）
   - **KDE提取**（最快）
   - **LibreOffice转换**（最兼容）
3. 点击"打开PPT文件"选择文件
4. 程序自动生成并显示缩略图

## 性能对比

| 方式 | 速度 | 质量 | 兼容性 | 依赖 |
|------|------|------|--------|------|
| KDE提取 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | unzip |
| LibreOffice | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | LibreOffice |
| 自动检测 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 两者 |

## 支持格式

### 完全支持（KDE提取）
- ✅ PPTX - PowerPoint 2007+
- ✅ DOCX - Word 2007+
- ✅ XLSX - Excel 2007+
- ✅ ODP - OpenDocument演示文稿
- ✅ ODT - OpenDocument文本
- ✅ ODS - OpenDocument电子表格

### 兼容支持（LibreOffice转换）
- ✅ PPT - PowerPoint 97-2003
- ✅ DOC - Word 97-2003
- ✅ XLS - Excel 97-2003
- ✅ 其他LibreOffice支持的格式

## 界面特性

- 🎨 现代化深色主题界面
- 📱 响应式网格布局（每行3个缩略图）
- 🖱️ 鼠标悬停效果
- 📊 实时进度显示
- 🔄 方法切换提示
- 📋 详细状态信息

## 技术架构

```
PPTThumbnailViewer/
├── src/
│   ├── main.cpp                 # 程序入口
│   ├── mainwindow.h/cpp         # 主窗口界面
│   ├── thumbnailgenerator.h/cpp # 缩略图生成器（协调器）
│   ├── thumbnailextractor.h/cpp # KDE风格提取器（新增）
│   └── thumbnailwidget.h/cpp    # 缩略图显示组件
├── PPTThumbnailViewer.pro       # qmake项目文件
└── README.md                    # 说明文档
```

## 开发说明

### 核心类说明

**ThumbnailGenerator**
- 主要协调器，管理两种生成方式
- 提供统一的接口和方法选择
- 处理临时文件和进度通知

**ThumbnailExtractor**（新增）
- 实现KDE风格的直接提取
- 处理ZIP文件解压和图像提取
- 支持Office Open XML和OpenDocument格式

**MainWindow**
- 用户界面管理
- 方法选择和状态显示
- 缩略图网格布局

### 扩展开发
- 可以添加更多文件格式支持
- 可以优化缓存机制
- 可以添加批量处理功能

## 版本历史

### v2.0（当前版本）
- ✨ 新增KDE风格缩略图提取
- 🚀 大幅提升生成速度
- 🎛️ 添加生成方式选择
- 📱 优化用户界面
- 🔧 改进错误处理

### v1.0
- 基础LibreOffice转换功能
- 简单的缩略图显示
- 基本的用户界面

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目！

---

**注意**：KDE风格提取方式模仿了KDE Dolphin文件管理器的实现原理，为用户提供了与系统一致的缩略图生成体验。 