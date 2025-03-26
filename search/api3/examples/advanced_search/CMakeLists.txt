find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(advanced_search 
    main.cpp
    advanced_search_window.h
    advanced_search_window.cpp
)

target_link_libraries(advanced_search PRIVATE 
    qsearch
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
) 