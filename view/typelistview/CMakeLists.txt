cmake_minimum_required(VERSION 3.16)

project(typelistview VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Core source files
set(CORE_SOURCES
    src/core/fileitem.cpp
)

set(CORE_HEADERS
    src/core/fileitem.h
    src/core/groupinfo.h
)

# Strategy implementations
set(STRATEGY_SOURCES
    src/strategies/groupingstrategy.cpp
    src/strategies/sortingstrategy.cpp
)

set(STRATEGY_HEADERS
    src/strategies/groupingstrategy.h
    src/strategies/sortingstrategy.h
)

# Model source files
set(MODEL_SOURCES
    src/models/filesystemmodel.cpp
)

set(MODEL_HEADERS
    src/models/filesystemmodel.h
)

# View source files
set(VIEW_SOURCES
    src/views/filelistview.cpp
    src/views/fileitemdelegate.cpp
    src/views/fileicondelegate.cpp
    src/views/selectionmanager.cpp
)

set(VIEW_HEADERS
    src/views/filelistview.h
    src/views/fileitemdelegate.h
    src/views/fileicondelegate.h
    src/views/selectionmanager.h
)

# Controller source files
set(CONTROLLER_SOURCES
    src/controllers/contextmenucontroller.cpp
)

set(CONTROLLER_HEADERS
    src/controllers/contextmenucontroller.h
)

# Main application files
set(MAIN_SOURCES
    main.cpp
    mainwindow.cpp
)

set(MAIN_HEADERS
    mainwindow.h
)

# Combine all sources
set(PROJECT_SOURCES
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${STRATEGY_SOURCES}
    ${STRATEGY_HEADERS}
    ${MODEL_SOURCES}
    ${MODEL_HEADERS}
    ${VIEW_SOURCES}
    ${VIEW_HEADERS}
    ${CONTROLLER_SOURCES}
    ${CONTROLLER_HEADERS}
    ${MAIN_SOURCES}
    ${MAIN_HEADERS}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(typelistview
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET typelistview APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(typelistview SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(typelistview
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(typelistview PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.typelistview)
endif()
set_target_properties(typelistview PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS typelistview
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(typelistview)
endif()
