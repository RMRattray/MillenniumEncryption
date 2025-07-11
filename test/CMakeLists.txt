add_executable(codebook_test 
    codebook_test.cpp
)

target_include_directories(codebook_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/crypt)

target_link_libraries(codebook_test PRIVATE millennium_crypt)