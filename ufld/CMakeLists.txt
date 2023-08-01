cmake_minimum_required(VERSION 3.26...3.26 FATAL_ERROR)
project(ufld CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.26: https://github.com/Kitware/CMake/blob/v3.26.4/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

find_package(OpenCV REQUIRED)
find_package(OnnxRuntime REQUIRED)
find_package(xtensor REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES
        FILES
        interface/math.ixx
        interface/ufld.ixx

        src/base.cppm
        src/ufld.cppm
        src/utils.cppm
        src/v1.cppm
        src/v1_config.cppm)

target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS} ${OnnxRuntime_INCLUDE_DIRS} ${xtensor_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES} ${xtensor_LIBRARIES} xtensor::optimize xtensor::use_xsimd)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

add_subdirectory(test)
