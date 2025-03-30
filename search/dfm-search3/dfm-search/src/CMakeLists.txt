set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/searchmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/searchquery.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/searchprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/searchresult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/filenamedataprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/filecontentdataprovider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/dfm-search/appdataprovider.h
)

set(SOURCE_FILES
    global.cpp
    searchmanager.cpp
    searchquery.cpp
    searchprovider.cpp
    searchresult.cpp
    filenamedataprovider.cpp
    filecontentdataprovider.cpp
    appdataprovider.cpp
)

add_library(dfm-search SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(dfm-search
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dfm-search
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

set_target_properties(dfm-search PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADER_FILES}"
)

install(TARGETS dfm-search
    EXPORT dfm-search-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/dfm-search
)

install(EXPORT dfm-search-targets
    FILE dfm-search-targets.cmake
    NAMESPACE DFMSearch::
    DESTINATION lib/cmake/dfm-search
) 