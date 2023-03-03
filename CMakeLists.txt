cmake_minimum_required(VERSION 3.22)
project(ETS2_Lane_Detection)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_subdirectory(d3d11_capture)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE d3d11_capture ufld ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
