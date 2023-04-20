cmake_minimum_required(VERSION 3.22)
project(ets2ld)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(imgui REQUIRED)
find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME}
        include/ets2ld/arguments.h                      src/arguments.cpp
        include/ets2ld/application.h                    src/application.cpp
        include/ets2ld/event.h                          src/event.cpp
        include/ets2ld/lane_detection_result_buffer.h   src/lane_detection_result_buffer.cpp
        include/ets2ld/ui.h                             src/ui.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} imgui::imgui dx11 ufld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
