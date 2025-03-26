find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(app_launcher main.cpp)

target_link_libraries(app_launcher PRIVATE 
    qsearch
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
) 