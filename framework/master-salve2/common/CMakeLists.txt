add_library(common INTERFACE
    message.h
    protocol.h
)

target_link_libraries(common INTERFACE
    Qt6::Core
)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}) 
