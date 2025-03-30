add_executable(master
    main.cpp
    master.cpp
    master.h
)

target_link_libraries(master PRIVATE
    Qt6::Core
    Qt6::Network
    common
)

target_include_directories(master PRIVATE
    ${CMAKE_SOURCE_DIR}/common
) 