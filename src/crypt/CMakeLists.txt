if (NOT TARGET millennium_crypt)
add_library(millennium_crypt STATIC
    encrypt.cpp
    encrypt.h
    codebook.cpp
    codebook.h
)
target_include_directories(millennium_crypt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()