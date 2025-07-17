add_executable(millennium_server
    main.cpp
)

target_link_libraries(millennium_server PRIVATE 
    millennium_crypt
    sqlite
)