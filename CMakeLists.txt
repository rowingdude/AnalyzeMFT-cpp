cmake_minimum_required(VERSION 3.16)

project(AnalyzeMFT
    VERSION 3.0.6.6
    DESCRIPTION "C++ NTFS Master File Table Analyzer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTING "Enable testing" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP support" OFF)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/analyzeMFT
)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    set(PLATFORM_LIBS)
elseif(UNIX)
    set(PLATFORM_LIBS pthread dl)
endif()

if(MSVC)
    add_compile_options(/W4)
    if(ENABLE_SIMD)
        add_compile_options(/arch:AVX2)
    endif()
else()
    add_compile_options(-Wall -Wextra -pedantic)
    if(ENABLE_SIMD)
        add_compile_options(-mavx2 -msse4.2)
    endif()
endif()

if(ENABLE_SIMD)
    include(CheckCXXCompilerFlag)
    
    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512)
    if(COMPILER_SUPPORTS_AVX512)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")
        add_definitions(-DAVX512_SUPPORTED)
    endif()
    
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        add_definitions(-DAVX2_SUPPORTED)
    endif()
    
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
        add_definitions(-DSSE42_SUPPORTED)
    endif()
    
    check_cxx_compiler_flag("-mfpu=neon" COMPILER_SUPPORTS_NEON)
    if(COMPILER_SUPPORTS_NEON)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
        add_definitions(-DNEON_SUPPORTED)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
else()
    add_definitions(-DNDEBUG)
    if(NOT MSVC)
        add_compile_options(-O3)
    endif()
endif()

find_package(PkgConfig QUIET)

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    add_definitions(-DHAVE_OPENSSL)
endif()

find_package(SQLite3 QUIET)
if(SQLite3_FOUND OR TARGET SQLite::SQLite3)
    add_definitions(-DHAVE_SQLITE3)
endif()

set(CORE_SOURCES
    src/core/winTime.cpp
    src/core/mftRecord.cpp
    src/core/mftAnalyzer.cpp
)

set(UTILS_SOURCES
    src/utils/stringUtils.cpp
    src/utils/logger.cpp
    src/utils/fsUtils.cpp
)

if(OpenSSL_FOUND)
    list(APPEND UTILS_SOURCES src/utils/hashCalc.cpp)
endif()

set(WRITERS_SOURCES
    src/writers/fileWriter.cpp
    src/writers/csvWriter.cpp
    src/writers/jsonWriter.cpp
    src/writers/xmlWriter.cpp
    src/writers/bodyWriter.cpp
    src/writers/timelineWriter.cpp
)

if(OpenSSL_FOUND)
    list(APPEND WRITERS_SOURCES src/writers/excelWriter.cpp)
endif()

if(SQLite3_FOUND OR TARGET SQLite::SQLite3)
    list(APPEND WRITERS_SOURCES src/writers/sqliteWriter.cpp)
endif()

set(PARSERS_SOURCES
    src/parsers/attributeParser.cpp
    src/parsers/standardinfoParser.cpp
    src/parsers/filenameParser.cpp
    src/parsers/objectidParser.cpp
    src/parsers/volumeParser.cpp
    src/parsers/dataParser.cpp
    src/parsers/indexParser.cpp
    src/parsers/bitmapParser.cpp
    src/parsers/reparsepointParser.cpp
    src/parsers/xattrParser.cpp
)

set(CLI_SOURCES
    src/cli/cliParser.cpp
    src/cli/app.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
    ${WRITERS_SOURCES}
    ${PARSERS_SOURCES}
    ${CLI_SOURCES}
)

add_library(libAnalyzeMFT ${ALL_SOURCES})

set_target_properties(libAnalyzeMFT PROPERTIES
    OUTPUT_NAME "analyzemft"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(OpenSSL_FOUND)
    target_link_libraries(libAnalyzeMFT OpenSSL::SSL OpenSSL::Crypto)
endif()

if(SQLite3_FOUND)
    target_link_libraries(libAnalyzeMFT SQLite::SQLite3)
elseif(TARGET SQLite3::SQLite3)
    target_link_libraries(libAnalyzeMFT SQLite3::SQLite3)
endif()

target_link_libraries(libAnalyzeMFT ${PLATFORM_LIBS})

add_executable(analyzemft src/main.cpp)
target_link_libraries(analyzemft libAnalyzeMFT)

install(TARGETS analyzemft libAnalyzeMFT
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)