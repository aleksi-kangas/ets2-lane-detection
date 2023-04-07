cmake_minimum_required(VERSION 3.22)
project(ufld)

option(BUILD_UFLD_TESTS "Build UFLD tests" ${BUILD_TESTS})

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(OpenCV REQUIRED)
find_package(OnnxRuntime REQUIRED)

add_library(${PROJECT_NAME}
        include/ufld/ufld.h         src/ufld.cpp
        include/ufld/utils.h        src/utils.cpp
        include/ufld/v1.h           src/v1.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include ${OpenCV_INCLUDE_DIRS} ${OnnxRuntime_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if (${BUILD_UFLD_TESTS})
    add_subdirectory(test)
endif ()
