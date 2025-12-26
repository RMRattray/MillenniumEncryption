# Check for various versions of SQLite3
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3510100" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3510100")
    set(SQLITE_DIR "sqlite-amalgamation-3510100")
elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3500300" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3500300")
    set(SQLITE_DIR "sqlite-amalgamation-3500300")
else()
    message(SEND_ERROR "SQLite3 not found.  Go to sqlite3.org, download the source code as an amalgamation, and extract it in the 'third_party' folder.")
endif()

# Link SQLite3 as static library
add_library(sqlite STATIC 
    "${SQLITE_DIR}/sqlite3.c"
    "${SQLITE_DIR}/shell.c"
)
target_include_directories(sqlite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/${SQLITE_DIR}")