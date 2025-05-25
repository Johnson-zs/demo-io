# PPT缩略图查看器

基于Qt6和CMake开发的PowerPoint文件缩略图生成和查看工具。

## 功能特性

- 🎯 支持.ppt和.pptx格式文件
- 🖼️ 高质量缩略图生成
- 🎨 现代化暗色主题界面
- ⚡ 快速预览和浏览
- 🔄 实时生成进度显示
- 💫 优雅的悬停效果和动画

## 系统要求

### 必需依赖
- **Qt6** (6.2+)
- **CMake** (3.16+)
- **LibreOffice** (用于PPT文件转换)
- **C++17兼容编译器**

### 支持的系统
- Linux (测试环境: Ubuntu 22.04+)
- macOS (理论支持)
- Windows (理论支持)

## 安装依赖

### Ubuntu/Debian系统
```bash
# 安装Qt6开发库
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# 安装LibreOffice (缩略图生成必需)
sudo apt install libreoffice

# 安装额外的Qt6模块
sudo apt install qt6-base-dev-tools libqt6core6 libqt6gui6 libqt6widgets6
```

### Arch Linux
```bash
# 安装Qt6和CMake
sudo pacman -S qt6-base qt6-tools cmake gcc

# 安装LibreOffice
sudo pacman -S libreoffice-fresh
```

### macOS (使用Homebrew)
```bash
# 安装Qt6和CMake
brew install qt6 cmake

# 安装LibreOffice
brew install --cask libreoffice
```

## 编译和运行

### 1. 克隆或下载项目
```bash
# 如果项目在git仓库中
git clone <项目地址>
cd ppt-thumbnail-viewer

# 或者直接使用现有的项目目录
cd /path/to/your/project
```

### 2. 创建构建目录
```bash
mkdir build
cd build
```

### 3. 配置CMake
```bash
# 基本配置
cmake ..

# 或者指定Qt6路径（如果自动检测失败）
cmake -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6 ..

# Debug版本
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

### 4. 编译项目
```bash
# 使用所有可用CPU核心编译
make -j$(nproc)

# 或者单核编译（如果内存不足）
make
```

### 5. 运行程序
```bash
./PPTThumbnailViewer
```

## 使用说明

### 基本操作
1. **打开文件**: 点击"打开PPT文件"按钮或使用快捷键`Ctrl+O`
2. **选择PPT文件**: 支持.ppt和.pptx格式
3. **等待生成**: 程序会自动调用LibreOffice生成缩略图
4. **浏览缩略图**: 在网格视图中查看所有页面
5. **交互操作**: 
   - 单击缩略图查看页面信息
   - 双击缩略图执行特殊操作（可扩展）
   - 悬停查看高亮效果

### 菜单功能
- **文件菜单**:
  - 打开PPT文件 (Ctrl+O)
  - 清除缩略图 (Ctrl+D)
  - 退出程序 (Ctrl+Q)
- **帮助菜单**:
  - 关于程序

## 技术架构

### 核心组件
1. **MainWindow**: 主界面管理
2. **ThumbnailGenerator**: 缩略图生成器（调用LibreOffice）
3. **ThumbnailWidget**: 缩略图显示组件

### 技术栈
- **界面框架**: Qt6 Widgets
- **构建系统**: CMake
- **文件转换**: LibreOffice Headless模式
- **图像处理**: Qt6内置图像库
- **异步处理**: Qt6信号槽机制

### 工作流程
```
用户选择PPT文件 
    ↓
调用LibreOffice转换为PNG
    ↓
监控临时目录文件生成
    ↓
逐个加载并显示缩略图
    ↓
用户交互和浏览
```

## 故障排除

### 常见问题

**Q: 提示"LibreOffice未安装或不可用"**
```bash
# 检查LibreOffice是否安装
libreoffice --version

# 如果未安装，请安装LibreOffice
sudo apt install libreoffice  # Ubuntu/Debian
```

**Q: CMake找不到Qt6**
```bash
# 设置Qt6路径
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6

# 或者在cmake命令中指定
cmake -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6 ..
```

**Q: 编译时提示头文件缺失**
```bash
# 安装完整的Qt6开发包
sudo apt install qt6-base-dev qt6-tools-dev

# 检查pkg-config
pkg-config --modversion Qt6Core Qt6Widgets Qt6Gui
```

**Q: 程序运行时崩溃**
```bash
# 以Debug模式运行
gdb ./PPTThumbnailViewer

# 或者查看输出日志
./PPTThumbnailViewer 2>&1 | tee debug.log
```

### 环境变量设置
```bash
# 如果Qt6安装在非标准位置
export Qt6_DIR=/opt/qt6/lib/cmake/Qt6
export PATH=/opt/qt6/bin:$PATH
export LD_LIBRARY_PATH=/opt/qt6/lib:$LD_LIBRARY_PATH
```

## 开发和扩展

### 添加新功能
程序采用模块化设计，可以方便地扩展：

1. **缩略图生成器扩展**: 修改`ThumbnailGenerator`类支持其他格式
2. **界面美化**: 在`ThumbnailWidget`中添加更多视觉效果
3. **功能增强**: 在`MainWindow`中添加导出、打印等功能

### 代码结构
```
src/
├── main.cpp              # 程序入口
├── mainwindow.h/cpp      # 主窗口
├── thumbnailgenerator.h/cpp  # 缩略图生成器
└── thumbnailwidget.h/cpp # 缩略图组件
```

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来帮助改进这个项目！

---

*基于Qt6开发，现代化、高效、易用的PPT缩略图查看工具。* 