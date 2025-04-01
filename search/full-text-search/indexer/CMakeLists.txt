add_executable(text-indexer main.cpp)

target_link_libraries(text-indexer PRIVATE
    Qt6::Core
    PkgConfig::Lucene
)

install(TARGETS text-indexer
    RUNTIME DESTINATION bin
) 