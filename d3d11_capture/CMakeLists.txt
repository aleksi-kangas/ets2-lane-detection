# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.22)
project(d3d11_capture)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME}
        include/d3d11_capture/device.h src/device.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)