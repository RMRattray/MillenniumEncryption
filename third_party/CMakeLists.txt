set(SQLITE_DIR "sqlite-amalgamation-3510100")

add_library(sqlite STATIC 
    "${SQLITE_DIR}/sqlite3.c"
    "${SQLITE_DIR}/shell.c"
)
target_include_directories(sqlite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/${SQLITE_DIR}")