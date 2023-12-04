cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR)
project(ufld_test CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.27: https://github.com/Kitware/CMake/blob/v3.27.9/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

include(FetchContent)
set(FETCHCONTENT_QUIET)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG main)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(${PROJECT_NAME} math_test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ufld gtest gtest_main)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

gtest_discover_tests(${PROJECT_NAME})
