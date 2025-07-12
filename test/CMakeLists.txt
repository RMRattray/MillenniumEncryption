add_executable(codebook_test 
    codebook_test.cpp
)

add_executable(encryption_test
    encryption_test.cpp
)

target_include_directories(codebook_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/crypt)
target_include_directories(encryption_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/crypt)

target_link_libraries(codebook_test PRIVATE millennium_crypt)
target_link_libraries(encryption_test PRIVATE millennium_crypt)