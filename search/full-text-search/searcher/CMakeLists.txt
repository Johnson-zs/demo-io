add_executable(text-searcher main.cpp)

target_link_libraries(text-searcher PRIVATE
    Qt6::Core
    PkgConfig::Lucene
)

install(TARGETS text-searcher
    RUNTIME DESTINATION bin
) 