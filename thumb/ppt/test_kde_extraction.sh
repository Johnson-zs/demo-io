#!/bin/bash

echo "=== PPT缩略图查看器 v2.0 测试脚本 ==="
echo

# 检查程序是否存在
if [ ! -f "./build/PPTThumbnailViewer" ]; then
    echo "❌ 程序未找到，请先编译："
    echo "   qmake6 PPTThumbnailViewer.pro && make"
    exit 1
fi

echo "✅ 程序文件存在"

# 检查依赖
echo
echo "🔍 检查系统依赖..."

# 检查unzip
if command -v unzip >/dev/null 2>&1; then
    echo "✅ unzip 命令可用"
else
    echo "❌ unzip 命令不可用，KDE提取功能将无法工作"
fi

# 检查LibreOffice
if command -v libreoffice >/dev/null 2>&1; then
    echo "✅ LibreOffice 可用"
    libreoffice --version | head -1
else
    echo "⚠️  LibreOffice 不可用，兼容性模式将无法工作"
fi

# 检查Qt6
echo
echo "🔍 检查Qt6环境..."
if command -v qmake6 >/dev/null 2>&1; then
    echo "✅ Qt6 开发环境可用"
    qmake6 --version | head -1
else
    echo "❌ Qt6 开发环境不可用"
fi

echo
echo "🚀 功能特性说明："
echo "1. KDE风格提取 - 直接从文件提取内嵌缩略图（极速）"
echo "2. LibreOffice转换 - 通用转换方式（兼容性好）"
echo "3. 自动检测 - 智能选择最佳方式"

echo
echo "📁 支持的文件格式："
echo "   KDE提取: PPTX, DOCX, XLSX, ODP, ODT, ODS"
echo "   LibreOffice: PPT, DOC, XLS + 所有上述格式"

echo
echo "🎯 使用方法："
echo "1. 运行程序: ./build/PPTThumbnailViewer"
echo "2. 在工具栏选择生成方式"
echo "3. 点击'打开PPT文件'选择文件"
echo "4. 观察生成速度和质量差异"

echo
echo "💡 性能对比："
echo "   KDE提取:     ⭐⭐⭐⭐⭐ (极速)"
echo "   LibreOffice: ⭐⭐ (较慢)"
echo "   自动检测:    ⭐⭐⭐⭐ (智能)"

echo
echo "🔧 如果遇到问题："
echo "1. 确保有测试用的PPTX文件"
echo "2. 检查文件权限"
echo "3. 查看程序输出的调试信息"

echo
echo "准备启动程序..."
echo "按回车键继续，或Ctrl+C取消"
read -r

echo "🚀 启动PPT缩略图查看器..."
./build/PPTThumbnailViewer 