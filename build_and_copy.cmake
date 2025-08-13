# Process script arguments
# Adapted from MS repository
set(i "1")
while(i LESS "${CMAKE_ARGC}")
    if("${CMAKE_ARGV${i}}" STREQUAL "cnr")
        set(ARG_CLEAN_AND_RECONFIGURE "TRUE")
    endif()
    math(EXPR i "${i} + 1") # next argument
endwhile()

if (ARG_CLEAN_AND_RECONFIGURE)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/build)

    execute_process(
        COMMAND cmake -B build
    )
endif()

execute_process(
    COMMAND cmake --build .
    WORKING_DIRECTORY "build"
    RESULT_VARIABLE EXIT_STATUS
)
if(NOT "${EXIT_STATUS}" EQUAL "0")
    message(FATAL_ERROR "Build step failed with status ${EXIT_STATUS}. See output above for details.")
endif()

execute_process(
    COMMAND "cp Debug/*.exe aux_build"
    WORKING_DIRECTORY "build"
)