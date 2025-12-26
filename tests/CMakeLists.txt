# ============================================================================
# Test Dependencies
# ============================================================================
include(FetchContent)

# Catch2 for unit testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Python for integration tests
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# ============================================================================
# Unit Tests
# ============================================================================
# Automatically gather all unit test files
file(GLOB UNIT_TEST_SOURCES
    unit_tests.cpp
    unit/*.cpp
)

add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests 
    PRIVATE 
        Catch2::Catch2WithMain 
        tcp_server_lib
)

# Register unit tests with CTest
include(Catch)
catch_discover_tests(unit_tests)

# ============================================================================
# Integration Tests
# ============================================================================
# Automatically discover Python test files
file(GLOB INTEGRATION_TESTS
    integration/test_*.py
    run_tests.py
)

foreach(TEST_FILE ${INTEGRATION_TESTS})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_test(
        NAME integration_${TEST_NAME}
        COMMAND ${Python3_EXECUTABLE} ${TEST_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(integration_${TEST_NAME} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}"
    )
endforeach()

# ============================================================================
# Test Utilities
# ============================================================================
# Custom target to run all tests with verbose output
add_custom_target(test_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure
    DEPENDS unit_tests tcp_server
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
