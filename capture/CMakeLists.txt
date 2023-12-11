# This screen capture library is highly based on a Python library
#   'DXcam' by 'ra1nty' (https://github.com/ra1nty/DXcam).
# The original library is licensed under the MIT License.
# This library is also licensed under the MIT License.

cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(capture CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
        src/camera.ixx
        src/capture.ixx
        src/device.ixx
        src/duplicator.ixx
        src/output.ixx
        src/surface.ixx
        src/timer.ixx
        src/utils.ixx)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
