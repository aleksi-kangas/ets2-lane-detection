# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.22)
project(d3d11_capture)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME}
        include/d3d11_capture/camera.h          src/camera.cpp
        include/d3d11_capture/common.h          src/common.cpp
        include/d3d11_capture/device.h          src/device.cpp
        include/d3d11_capture/duplicator.h      src/duplicator.cpp
        include/d3d11_capture/event.h           src/event.cpp
        include/d3d11_capture/factory.h         src/factory.cpp
        include/d3d11_capture/frame_buffer.h    src/frame_buffer.cpp
        include/d3d11_capture/output.h          src/output.cpp
        include/d3d11_capture/region.h          src/region.cpp
        include/d3d11_capture/stage_surface.h   src/stage_surface.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
