cmake_minimum_required(VERSION 3.25)
project(ets2ld)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.25
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(imgui REQUIRED)
find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        interface/application.ixx
        interface/ets2ld.ixx
        interface/settings.ixx
        interface/ui.ixx
        interface/utils.ixx)
target_sources(${PROJECT_NAME} PRIVATE
        src/application.cppm
        src/ui.cppm
        src/utils.cppm)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} imgui::imgui capture ufld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
