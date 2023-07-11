cmake_minimum_required(VERSION 3.26...3.26 FATAL_ERROR)
project(ufld_test CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.26: https://github.com/Kitware/CMake/blob/v3.26.4/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG main)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        math_test.cppm)
target_link_libraries(${PROJECT_NAME} PUBLIC ufld gtest gtest_main)
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

gtest_discover_tests(${PROJECT_NAME})
