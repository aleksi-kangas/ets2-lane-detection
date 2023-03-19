# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.22)
project(dx11)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME}
        include/dx11/camera.h          src/camera.cpp
        include/dx11/capture.h         src/capture.cpp
        include/dx11/common.h          src/common.cpp
        include/dx11/device.h          src/device.cpp
        include/dx11/duplicator.h      src/duplicator.cpp
        include/dx11/event.h           src/event.cpp
        include/dx11/frame_buffer.h    src/frame_buffer.cpp
        include/dx11/output.h          src/output.cpp
        include/dx11/region.h          src/region.cpp
        include/dx11/surface.h         src/surface.cpp
        include/dx11/timer.h           src/timer.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
