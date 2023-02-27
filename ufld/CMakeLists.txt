cmake_minimum_required(VERSION 3.22)
project(ufld)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(OnnxRuntime REQUIRED)

add_library(${PROJECT_NAME}
        include/ufld/ufld.h src/ufld.cpp
        include/ufld/v1.h src/v1.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS} ${onnxruntime_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${onnxruntime_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
