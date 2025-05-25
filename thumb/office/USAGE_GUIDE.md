# Office Preview 使用指南

## 🎉 项目构建成功！

恭喜！Office文件快速预览程序已成功构建并可以运行。

## 🚀 快速开始

### 启动程序
```bash
cd /home/zhangs/Code/github/demo-io/thumb/office
./build/bin/OfficePreview
```

### 测试文件
我们已经为你准备了一个测试文档：
- `test_document.docx` - Word格式测试文档

## 💡 主要功能

### 1. 打开文件的方式
- **菜单**: 文件 → 打开文件 (Ctrl+O)
- **工具栏**: 点击"打开文件"按钮
- **拖拽**: 直接将Office文件拖入程序窗口
- **最近文件**: 从左侧面板双击最近打开的文件

### 2. 预览控制
- **翻页**: 使用 ◀上一页 / 下一页▶ 按钮，或直接输入页码
- **缩放**: 
  - 使用缩放滑块进行精确控制
  - 点击"适应窗口"自动缩放到合适大小
  - 点击"实际大小"显示100%大小
- **界面**: 现代化深色主题，护眼舒适

### 3. 支持的文件格式
- **Word**: .docx, .doc
- **Excel**: .xlsx, .xls  
- **PowerPoint**: .pptx, .ppt

## 🔧 技术特性

### 智能缓存
- 首次转换的文件会被缓存，再次打开速度更快
- 缓存位置：`~/.cache/Office Preview/office_preview/`

### 高质量渲染
- 基于Poppler-Qt6的高质量PDF渲染
- 支持反锯齿和文本反锯齿
- 可任意缩放而不失真

### 性能优化
- LibreOffice headless模式转换，资源占用小
- 异步处理，界面不卡顿
- 进度显示，转换状态一目了然

## 📋 快速测试步骤

1. **启动程序**:
   ```bash
   ./build/bin/OfficePreview
   ```

2. **测试拖拽功能**:
   - 将 `test_document.docx` 拖拽到程序窗口
   - 观察转换进度和预览效果

3. **测试导航功能**:
   - 如果文档有多页，测试翻页功能
   - 尝试不同的缩放选项

4. **测试最近文件**:
   - 打开几个不同的Office文件
   - 查看左侧最近文件列表是否正确显示

## 🐛 故障排除

### 如果遇到"LibreOffice未安装"错误
```bash
# 检查LibreOffice安装
libreoffice --version

# 如未安装，执行安装
sudo apt install libreoffice
```

### 如果程序无法启动
```bash
# 检查依赖
ldd build/bin/OfficePreview

# 重新构建
./build.sh
```

### 如果转换失败
- 确保Office文件未损坏
- 检查文件权限
- 查看系统缓存空间是否充足

## 📈 下一步扩展

你可以考虑添加以下功能：
- 支持更多文件格式（如RTF、ODT等）
- 添加打印功能
- 增加文档搜索功能
- 支持文档注释显示
- 添加全屏预览模式

## 🎯 项目总结

✅ **完成的功能**:
- ✅ 基于CMake的Qt6 C++17项目结构
- ✅ LibreOffice headless文件转换
- ✅ Poppler-Qt6高质量PDF渲染
- ✅ 现代化深色主题界面
- ✅ 拖拽文件支持
- ✅ 智能缓存机制
- ✅ 多页导航和缩放控制
- ✅ 最近文件列表
- ✅ 错误处理和进度显示

🔧 **技术栈**:
- Qt6 Widgets (C++17)
- CMake 3.16+ 构建系统
- Poppler-Qt6 PDF渲染
- LibreOffice Headless 转换
- 现代C++智能指针管理

这是一个功能完整、架构清晰的Office文件预览程序，可以作为你的项目基础进行进一步开发！ 