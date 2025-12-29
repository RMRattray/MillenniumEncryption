add_library(millennium_server
    packet.cpp
    packet.h
)

target_include_directories(millennium_server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(millennium_falcon
    main.cpp
    server.cpp
    server.h
)

target_link_libraries(millennium_falcon PRIVATE 
    millennium_crypt
    millennium_server
    sqlite
)

if (WIN_32) 
    target_link_libraries(millennium_falcon PRIVATE
        ws2_32
    )
endif()