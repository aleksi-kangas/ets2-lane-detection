cmake_minimum_required(VERSION 3.25)
project(ufld CXX)

option(BUILD_UFLD_TESTS "Build UFLD tests" ${BUILD_TESTS})

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.25
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(OpenCV REQUIRED)
find_package(OnnxRuntime REQUIRED)
find_package(xtensor REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        interface/base.ixx
        interface/math.ixx
        interface/ufld.ixx
        interface/utils.ixx
        interface/v1.ixx
        interface/v1_config.ixx)
target_sources(${PROJECT_NAME} PRIVATE
        src/base.cppm
        src/math.cppm
        src/ufld.cppm
        src/utils.cppm
        src/v1.cppm)
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS} ${OnnxRuntime_INCLUDE_DIRS} ${xtensor_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES} ${xtensor_LIBRARIES} xtensor::optimize xtensor::use_xsimd)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if (${BUILD_UFLD_TESTS})
    add_subdirectory(test)
endif ()
