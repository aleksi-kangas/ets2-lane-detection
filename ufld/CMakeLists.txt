cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR)
project(ufld CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.27: https://github.com/Kitware/CMake/blob/v3.27.9/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

# Copy the OnnxRuntime cmake files to the vcpkg installed directory
# These are needed to make the find_package(OnnxRuntime) work
file(COPY
        "${CMAKE_CURRENT_LIST_DIR}/cmake/onnxruntimeConfig.cmake"
        "${CMAKE_CURRENT_LIST_DIR}/cmake/onnxruntimeVersion.cmake"
        DESTINATION "${VCPKG_INSTALLED_DIR}/x64-windows/share/onnxruntime-gpu")

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
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES} xtensor xtensor::optimize xtensor::use_xsimd)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

add_subdirectory(test)
