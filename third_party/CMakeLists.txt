set(SQLITE_DIR "sqlite-amalgamation-3500300")

add_library(sqlite STATIC 
    "${SQLITE_DIR}/shell.c"
    "${SQLITE_DIR}/sqlite3.c"
    "${SQLITE_DIR}/sqlite3.h"
    "${SQLITE_DIR}/sqlite3ext.h"
)
target_include_directories(sqlite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/${SQLITE_DIR}")