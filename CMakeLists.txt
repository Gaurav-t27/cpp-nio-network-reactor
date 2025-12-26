cmake_minimum_required(VERSION 3.16)
project(cpp-nio-network-reactor VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# Build Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linux-only project - no Windows support
if(WIN32)
    message(FATAL_ERROR "This project only supports Linux builds")
endif()

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example applications" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Compiler Flags (Linux GCC/Clang)
# ============================================================================
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# ============================================================================
# Dependencies
# ============================================================================
find_package(Threads REQUIRED)

# ============================================================================
# Project Structure
# ============================================================================
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# Installation
# ============================================================================
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ 
        DESTINATION include/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.hpp")
