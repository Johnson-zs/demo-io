#!/bin/bash

# Office Preview 构建脚本

set -e  # 遇到错误立即退出

echo "=== Office Preview 构建脚本 ==="

# 检查依赖
echo "检查系统依赖..."

# 检查CMake
if ! command -v cmake &> /dev/null; then
    echo "错误: 未找到 CMake。请先安装 CMake 3.16+"
    exit 1
fi

# 检查Qt6
if ! pkg-config --exists Qt6Core; then
    echo "错误: 未找到 Qt6。请先安装 Qt6 开发包"
    echo "Ubuntu/Debian: sudo apt install qt6-base-dev"
    echo "Fedora: sudo dnf install qt6-qtbase-devel"
    exit 1
fi

# 检查Poppler-Qt6
if ! pkg-config --exists poppler-qt6; then
    echo "错误: 未找到 Poppler-Qt6。请先安装 Poppler-Qt6 开发包"
    echo "Ubuntu/Debian: sudo apt install libpoppler-qt6-dev"
    echo "Fedora: sudo dnf install poppler-qt6-devel"
    exit 1
fi

# 检查LibreOffice
if ! command -v libreoffice &> /dev/null; then
    echo "警告: 未找到 LibreOffice。程序需要 LibreOffice 进行文件转换"
    echo "Ubuntu/Debian: sudo apt install libreoffice"
    echo "Fedora: sudo dnf install libreoffice"
    echo "继续构建但运行时可能出错..."
fi

echo "依赖检查完成。"

# 创建构建目录
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "清理旧的构建目录..."
    rm -rf "$BUILD_DIR"
fi

echo "创建构建目录: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置项目
echo "配置项目..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"

# 检测Qt6路径
QT6_PATH=$(pkg-config --variable=prefix Qt6Core)
if [ -n "$QT6_PATH" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_PREFIX_PATH=$QT6_PATH"
    echo "找到Qt6安装路径: $QT6_PATH"
fi

cmake .. $CMAKE_ARGS

# 编译项目
echo "开始编译..."
CPU_COUNT=$(nproc 2>/dev/null || echo "4")
echo "使用 $CPU_COUNT 个CPU核心进行编译"

make -j$CPU_COUNT

echo ""
echo "=== 构建完成! ==="
echo ""
echo "可执行文件位置: $(pwd)/bin/OfficePreview"
echo ""
echo "运行程序:"
echo "  cd $(pwd)"
echo "  ./bin/OfficePreview"
echo ""
echo "或者直接运行:"
echo "  $(pwd)/bin/OfficePreview" 