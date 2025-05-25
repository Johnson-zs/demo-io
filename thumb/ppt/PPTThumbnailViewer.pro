QT += core widgets gui concurrent

CONFIG += c++17

TARGET = PPTThumbnailViewer
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/thumbnailgenerator.cpp \
    src/thumbnailwidget.cpp \
    src/thumbnailextractor.cpp

HEADERS += \
    src/mainwindow.h \
    src/thumbnailgenerator.h \
    src/thumbnailwidget.h \
    src/thumbnailextractor.h

INCLUDEPATH += src

DESTDIR = build
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

QMAKE_CXXFLAGS += -Wall -Wextra

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
} 