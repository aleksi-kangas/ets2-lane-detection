# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR)
project(capture CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.27: https://github.com/Kitware/CMake/blob/v3.27.9/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
        interface/camera.ixx
        interface/capture.ixx
        interface/utils.ixx

        src/device.cppm
        src/duplicator.cppm
        src/frame_buffer.cppm
        src/output.cppm
        src/surface.cppm
        src/timer.cppm)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
