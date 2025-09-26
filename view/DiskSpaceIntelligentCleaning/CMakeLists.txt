cmake_minimum_required(VERSION 3.19)
project(DiskSpaceIntelligentCleaning LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets DBus)
find_package(dfm6-search REQUIRED)

qt_standard_project_setup()

qt_add_executable(DiskSpaceIntelligentCleaning
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    disklistwidget.cpp
    disklistwidget.h
    diskdetailwidget.cpp
    diskdetailwidget.h
    devicemanager.h devicemanager.cpp
    deviceutils.h deviceutils.cpp
)

target_link_libraries(DiskSpaceIntelligentCleaning
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::DBus
        dfm6-search
)

include(GNUInstallDirs)

install(TARGETS DiskSpaceIntelligentCleaning
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DiskSpaceIntelligentCleaning
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
