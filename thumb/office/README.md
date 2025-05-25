# Office Preview - Office文件快速预览工具

一个基于Qt6和LibreOffice的Office文件快速预览程序，支持Word、Excel、PowerPoint文件的快速预览。

## 功能特性

- **支持多种Office格式**：
  - Microsoft Word (.docx, .doc)
  - Microsoft Excel (.xlsx, .xls)
  - Microsoft PowerPoint (.pptx, .ppt)

- **现代化界面**：
  - 深色主题设计
  - 直观的工具栏操作
  - 拖拽文件支持

- **强大的预览功能**：
  - 高质量PDF渲染
  - 灵活的缩放控制
  - 多页文档导航
  - 智能缓存机制

## 技术架构

- **Frontend**: Qt6 Widgets (C++17)
- **Build System**: CMake 3.16+
- **PDF Rendering**: Poppler-Qt6
- **Office Conversion**: LibreOffice Headless
- **Platform**: Linux, Windows, macOS

## 环境要求

### 系统依赖

1. **Qt6** (Core, Widgets, Gui)
2. **Poppler-Qt6** - PDF渲染库
3. **LibreOffice** - Office文件转换
4. **CMake** 3.16+
5. **C++17** 编译器

### Ubuntu/Debian 安装依赖

```bash
# 安装Qt6和开发工具
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# 安装Poppler-Qt6
sudo apt install libpoppler-qt6-dev

# 安装LibreOffice
sudo apt install libreoffice
```

### Fedora/CentOS 安装依赖

```bash
# 安装Qt6和开发工具
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

# 安装Poppler-Qt6
sudo dnf install poppler-qt6-devel

# 安装LibreOffice
sudo dnf install libreoffice
```

### macOS 安装依赖

```bash
# 使用Homebrew安装
brew install qt6 poppler-qt6 cmake libreoffice
```

### Windows 安装依赖

1. 安装Qt6 (从官网下载)
2. 安装LibreOffice
3. 编译或下载预编译的Poppler-Qt6
4. 安装Visual Studio或MinGW

## 编译构建

### Linux/macOS

```bash
# 克隆项目
git clone <project-url>
cd office

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 运行
./bin/OfficePreview
```

### Windows (Visual Studio)

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### Windows (MinGW)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## 使用方法

### 启动程序

```bash
./OfficePreview
```

### 打开文件

1. **菜单方式**: 文件 → 打开文件
2. **按钮方式**: 点击工具栏的"打开文件"按钮
3. **拖拽方式**: 直接将Office文件拖拽到程序窗口
4. **最近文件**: 从左侧面板选择最近打开的文件

### 预览操作

- **翻页**: 使用工具栏的上一页/下一页按钮，或直接输入页码
- **缩放**: 使用缩放滑块，或点击"适应窗口"/"实际大小"按钮
- **快捷键**: Ctrl+O (打开文件), Ctrl+Q (退出)

## 配置说明

### 缓存位置

程序会在以下位置创建缓存目录：
- Linux: `~/.cache/Office Preview/office_preview/`
- Windows: `%APPDATA%/Office Preview/cache/office_preview/`
- macOS: `~/Library/Caches/Office Preview/office_preview/`

### LibreOffice检测

程序会自动检测以下位置的LibreOffice安装：

**Linux**:
- `/usr/bin/libreoffice`
- `/usr/local/bin/libreoffice`
- `/opt/libreoffice/program/soffice`
- `/snap/bin/libreoffice`

**Windows**:
- `C:/Program Files/LibreOffice/program/soffice.exe`
- `C:/Program Files (x86)/LibreOffice/program/soffice.exe`

**macOS**:
- `/Applications/LibreOffice.app/Contents/MacOS/soffice`

## 故障排除

### 常见问题

1. **"LibreOffice 未安装或不可用"**
   - 确保已安装LibreOffice
   - 检查LibreOffice是否在系统PATH中
   - 尝试在终端运行 `libreoffice --version`

2. **"无法打开PDF文件"**
   - 确保已安装libpoppler-qt6-dev
   - 检查PDF文件是否损坏

3. **编译错误：找不到Qt6**
   - 设置Qt6安装路径：`export CMAKE_PREFIX_PATH=/path/to/qt6`
   - 或在cmake时指定：`cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..`

4. **编译错误：找不到Poppler**
   - Ubuntu: `sudo apt install libpoppler-qt6-dev`
   - 确保pkg-config可以找到poppler-qt6

### 性能优化

- 首次打开大文件时转换时间较长，后续会使用缓存
- 定期清理缓存目录以释放空间
- 关闭不需要的其他LibreOffice进程

## 开发信息

### 项目结构

```
office/
├── CMakeLists.txt          # 主构建配置
├── README.md              # 项目说明
├── src/                   # 源代码目录
│   ├── main.cpp          # 程序入口
│   ├── MainWindow.h/cpp  # 主窗口
│   ├── OfficePreviewWidget.h/cpp  # 预览控件
│   └── OfficeConverter.h/cpp      # 转换器
└── build/                 # 构建输出目录
```

### 代码特性

- C++17标准
- Qt6现代化API
- RAII资源管理
- 信号槽机制
- 异步文件处理

### 扩展开发

如需添加新功能或格式支持，主要修改以下文件：
- `OfficeConverter.cpp` - 添加新的转换逻辑
- `MainWindow.cpp` - 修改文件类型检测
- `OfficePreviewWidget.cpp` - 添加新的预览功能

## 许可证

本项目基于开源许可证发布，具体详情请查看LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目！ 