cmake_minimum_required(VERSION 3.16)

project(MillenniumEncryption VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Required for MSVC to properly work with Qt6
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

qt_standard_project_setup()

add_subdirectory(third_party)

add_subdirectory(src/app)
add_subdirectory(src/crypt)
add_subdirectory(src/loc)
add_subdirectory(src/server)

add_subdirectory(test)