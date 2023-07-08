cmake_minimum_required(VERSION 3.25)
project(ufld_test)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.25
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
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
