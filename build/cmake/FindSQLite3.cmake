# Find SQLite3
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SQLITE3 QUIET sqlite3)
endif()

find_path(SQLITE3_INCLUDE_DIR
    NAMES sqlite3.h
    PATHS ${PC_SQLITE3_INCLUDE_DIRS}
    PATH_SUFFIXES include
)

find_library(SQLITE3_LIBRARY
    NAMES sqlite3 libsqlite3
    PATHS ${PC_SQLITE3_LIBRARY_DIRS}
    PATH_SUFFIXES lib
)

if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
    set(SQLITE3_FOUND TRUE)
    set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})
    set(SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})
    
    if(NOT TARGET SQLite3::SQLite3)
        add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite3::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3
    REQUIRED_VARS SQLITE3_LIBRARIES SQLITE3_INCLUDE_DIRS
)

mark_as_advanced(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)