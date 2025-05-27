# Test configuration
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, downloading...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Unit tests
set(UNIT_TEST_SOURCES
    unit/testMFTRecord.cpp
    unit/testParsers.cpp
    unit/testWriters.cpp
    unit/windowsTime.cpp
)

add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests
    libAnalyzeMFT
    gtest_main
    gtest
)

# Integration tests
set(INTEGRATION_TEST_SOURCES
    integration/testFileFormats.cpp
    integration/testfullAnalysis.cpp
)

add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(integration_tests
    libAnalyzeMFT
    gtest_main
    gtest
)

# Register tests
add_test(NAME UnitTests COMMAND unit_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)

# Test data
add_custom_target(copy_test_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/tests/fixtures
    ${CMAKE_BINARY_DIR}/tests/fixtures
)

add_dependencies(unit_tests copy_test_data)
add_dependencies(integration_tests copy_test_data)