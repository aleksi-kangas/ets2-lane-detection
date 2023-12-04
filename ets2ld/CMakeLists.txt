cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR)
project(ets2ld CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.27: https://github.com/Kitware/CMake/blob/v3.27.9/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(imgui REQUIRED)
find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        interface/ets2ld.ixx

        src/application.cppm
        src/ui.cppm
        src/utils.cppm)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} imgui::imgui capture ufld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
