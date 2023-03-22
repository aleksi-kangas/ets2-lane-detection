cmake_minimum_required(VERSION 3.22)
project(ufld_test)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG main)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(${PROJECT_NAME} utils_test.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ufld gtest gtest_main)
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

gtest_discover_tests(${PROJECT_NAME})
