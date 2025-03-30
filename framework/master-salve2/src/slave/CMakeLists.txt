add_executable(slave
    main.cpp
    slave.cpp
    slave.h
    image_processor.h
    image_processor.cpp
    task_executor.cpp
)

target_link_libraries(slave PRIVATE
    Qt6::Core
    Qt6::Network
    common
)

target_include_directories(slave PRIVATE
    ${CMAKE_SOURCE_DIR}/common
) 