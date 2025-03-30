add_executable(dfm-search-cli
    main.cpp
)

target_link_libraries(dfm-search-cli
    PRIVATE
        dfm-search
        Qt6::Core
        Qt6::Widgets
)

install(TARGETS dfm-search-cli
    RUNTIME DESTINATION bin
) 