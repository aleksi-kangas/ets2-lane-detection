# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.22)
project(capture)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.25
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        interface/camera.ixx
        interface/capture.ixx
        interface/device.ixx
        interface/duplicator.ixx
        interface/frame_buffer.ixx
        interface/output.ixx
        interface/surface.ixx
        interface/timer.ixx
        interface/utils.ixx)
target_sources(${PROJECT_NAME} PRIVATE
        src/camera.cppm
        src/capture.cppm
        src/device.cppm
        src/duplicator.cppm
        src/frame_buffer.cppm
        src/output.cppm
        src/surface.cppm
        src/timer.cppm
        src/utils.cppm)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)