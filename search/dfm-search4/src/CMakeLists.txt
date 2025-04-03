# 源文件列表
set(SOURCES
    core/abstractsearchengine.cpp
    core/searchengine.cpp
    core/searchoptions.cpp
    core/searchoptionsdata.cpp
    core/searchresult.cpp
    core/searchquery.cpp
    core/searchprovider.cpp
    core/searchfactory.cpp
    
    filenamesearch/filenamesearchengine.cpp
    filenamesearch/filenamesearchoptions.cpp
    filenamesearch/filenamesearchapi.cpp

    contentsearch/contentsearchengine.cpp
    contentsearch/contentsearchoptions.cpp
    contentsearch/contentsearchapi.cpp

    desktopsearch/desktopsearchengine.cpp
    desktopsearch/desktopsearchoptions.cpp
    desktopsearch/desktopsearchapi.cpp

    utils/searchutility.cpp
)

# 头文件列表
set(HEADERS
    core/abstractsearchengine.h
    core/searchoptionsdata.h
    core/searchresultdata.h
    
    filenamesearch/filenamesearchengine.h
    filenamesearch/filenamesearchoptions.h
    
    contentsearch/contentsearchengine.h
    contentsearch/contentsearchoptions.h
    
    desktopsearch/desktopsearchengine.h
    desktopsearch/desktopsearchoptions.h
    
    utils/searchutility.h
)

# 公共接口头文件
set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/global.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchengine.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchoptions.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchresult.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchquery.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchprovider.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/searchfactory.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/filenamesearchapi.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/contentsearchapi.h
    ${CMAKE_SOURCE_DIR}/include/dfm6-search/desktopsearchapi.h
)

# 添加库定义
add_library(dfm6-search SHARED ${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})

# 添加定义和包含路径
target_compile_definitions(dfm6-search PRIVATE DFM6_SEARCH_LIBRARY)
target_include_directories(dfm6-search 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接Qt
target_link_libraries(dfm6-search
    PUBLIC 
        Qt6::Core
)

# 安装库
install(TARGETS dfm6-search
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
) 
