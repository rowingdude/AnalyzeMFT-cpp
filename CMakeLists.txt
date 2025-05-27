cmake_minimum_required(VERSION 3.16)

project(AnalyzeMFT
    VERSION 3.0.6.6
    DESCRIPTION "C++ NTFS Master File Table Analyzer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include build configuration
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build/cmake")

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Project options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTING "Enable testing" ON)
option(ENABLE_BENCHMARKING "Enable benchmarking" OFF)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP support" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
option(INSTALL_DEVELOPMENT "Install development files" ON)

# Include build modules
include(CompileFlags)
include(FindDependencies)

# Set include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    set(PLATFORM_LIBS ws2_32 advapi32)
elseif(UNIX)
    set(PLATFORM_LIBS pthread dl)
endif()

# Compiler-specific flags
if(MSVC)
    add_compile_options(/W4 /WX)
    if(ENABLE_SIMD)
        add_compile_options(/arch:AVX2)
    endif()
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
    if(ENABLE_SIMD)
        add_compile_options(-mavx2 -msse4.2)
    endif()
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG -D_DEBUG)
else()
    add_definitions(-DNDEBUG)
    if(NOT MSVC)
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

if(ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(AnalyzeMFT OpenMP::OpenMP_CXX)
    endif()
endif()

# Source files
set(CORE_SOURCES
    src/core/winTime.cpp
    src/core/mftRecord.cpp
    src/core/mftAnalyzer.cpp
)

set(UTILS_SOURCES
    src/utils/hashCalc.cpp
    src/utils/stringUtils.cpp
    src/utils/logger.cpp
    src/utils/memUtils.cpp
    src/utils/fsUtils.cpp
)

set(WRITERS_SOURCES
    src/writers/fileWriter.cpp
    src/writers/csvWriter.cpp
    src/writers/jsonWriter.cpp
    src/writers/xmlWriter.cpp
    src/writers/excelWriter.cpp
    src/writers/sqliteWriter.cpp
    src/writers/bodyWriter.cpp
    src/writers/timelineWriter.cpp
)

set(PARSERS_SOURCES
    src/parsers/attributeParser.cpp
    src/parsers/standardinfoParser.cpp
    src/parsers/filenameParser.cpp
    src/parsers/objectidParser.cpp
    src/parsers/secdescParser.cpp
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

# Create library
add_library(libAnalyzeMFT ${ALL_SOURCES})

# Set library properties
set_target_properties(libAnalyzeMFT PROPERTIES
    OUTPUT_NAME "analyzemft"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Link libraries
target_link_libraries(libAnalyzeMFT
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite3::SQLite3
    ${PLATFORM_LIBS}
)

# Create executable
add_executable(analyzemft src/main.cpp)
target_link_libraries(analyzemft libAnalyzeMFT)

# C API library
add_library(canalyzemft include/canalyzemft.cpp)
target_link_libraries(canalyzemft libAnalyzeMFT)

# Install targets
install(TARGETS analyzemft libAnalyzeMFT canalyzemft
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(INSTALL_DEVELOPMENT)
    install(DIRECTORY include/ DESTINATION include)
    install(DIRECTORY src/ DESTINATION include/analyzeMFT
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Install data files
install(DIRECTORY data/ DESTINATION share/analyzemft/data)

# Testing
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Packaging
set(CPACK_PACKAGE_NAME "AnalyzeMFT")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "AnalyzeMFT Project")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)