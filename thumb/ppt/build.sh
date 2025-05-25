#!/bin/bash

# PPT缩略图查看器构建脚本

set -e

echo "正在构建PPT缩略图查看器..."

# 创建构建目录
mkdir -p build
cd build

# 清理之前的构建
rm -f *.o PPTThumbnailViewer moc_*.cpp

# Qt6路径
QT6_INCLUDE="/usr/include/x86_64-linux-gnu/qt6"
QT6_LIB="/usr/lib/x86_64-linux-gnu"

# 检查Qt6头文件
if [ ! -d "$QT6_INCLUDE" ]; then
    echo "错误：找不到Qt6头文件目录: $QT6_INCLUDE"
    echo "请安装Qt6开发包: sudo apt install qt6-base-dev"
    exit 1
fi

# 编译选项
CXXFLAGS="-std=c++17 -Wall -Wextra -fPIC"
INCLUDES="-I../src -I$QT6_INCLUDE -I$QT6_INCLUDE/QtCore -I$QT6_INCLUDE/QtWidgets -I$QT6_INCLUDE/QtGui"
LIBS="-L$QT6_LIB -lQt6Core -lQt6Widgets -lQt6Gui"
DEFINES="-DQT_CORE_LIB -DQT_GUI_LIB -DQT_WIDGETS_LIB"

echo "生成MOC文件..."

# 生成MOC文件
/usr/lib/qt6/libexec/moc $INCLUDES ../src/mainwindow.h -o moc_mainwindow.cpp
/usr/lib/qt6/libexec/moc $INCLUDES ../src/thumbnailgenerator.h -o moc_thumbnailgenerator.cpp  
/usr/lib/qt6/libexec/moc $INCLUDES ../src/thumbnailwidget.h -o moc_thumbnailwidget.cpp

echo "编译源文件..."

# 编译源文件
g++ $CXXFLAGS $INCLUDES $DEFINES -c ../src/main.cpp -o main.o
g++ $CXXFLAGS $INCLUDES $DEFINES -c ../src/mainwindow.cpp -o mainwindow.o
g++ $CXXFLAGS $INCLUDES $DEFINES -c ../src/thumbnailgenerator.cpp -o thumbnailgenerator.o
g++ $CXXFLAGS $INCLUDES $DEFINES -c ../src/thumbnailwidget.cpp -o thumbnailwidget.o

echo "编译MOC文件..."

# 编译MOC文件
g++ $CXXFLAGS $INCLUDES $DEFINES -c moc_mainwindow.cpp -o moc_mainwindow.o
g++ $CXXFLAGS $INCLUDES $DEFINES -c moc_thumbnailgenerator.cpp -o moc_thumbnailgenerator.o
g++ $CXXFLAGS $INCLUDES $DEFINES -c moc_thumbnailwidget.cpp -o moc_thumbnailwidget.o

echo "链接可执行文件..."

# 链接
g++ *.o $LIBS -o PPTThumbnailViewer

echo "构建完成！可执行文件: build/PPTThumbnailViewer"
echo ""
echo "运行程序："
echo "cd build && ./PPTThumbnailViewer" 