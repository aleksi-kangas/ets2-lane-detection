cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(ets2ld CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(imgui REQUIRED)
find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        src/application.ixx
        src/ets2ld.ixx
        src/ui.ixx
        src/utils.ixx)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} imgui::imgui capture ufld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
