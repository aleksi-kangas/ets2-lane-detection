cmake_minimum_required(VERSION 3.22)
project(ETS2_Lane_Detection)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(OpenCV REQUIRED)

add_subdirectory(dx11)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE dx11 ufld ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
