add_library(millennium_crypt STATIC
    encrypt.c
)

target_include_directories(millennium_crypt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})