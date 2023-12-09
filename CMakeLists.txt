cmake_minimum_required(VERSION 3.18)

project(poolparty VERSION 1.0.0
                  DESCRIPTION "A modern, fully tested C++ thread pool implementation"
                  LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Adding the cmake folder to the cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add most pedantic compiler flags
set(WARNING_FLAGS -Wall -Werror -Wextra -Wpedantic)

# Activate clang tidy
set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy)

add_library(pool_party INTERFACE)
target_include_directories(pool_party INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Including function to use git submodules
include(InitGitSubmodule)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    init_git_submodule(extern/googletest)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()
