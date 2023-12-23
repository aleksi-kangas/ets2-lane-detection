cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(ufld CXX)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

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
        src/core.ixx
        src/draw.ixx
        src/math.ixx
        src/ufld.ixx
        src/v1.ixx
        src/v1_config.ixx
        src/v2.ixx
        src/v2_config.ixx)

target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS} ${OnnxRuntime_INCLUDE_DIRS} ${xtensor_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBRARIES} ${OnnxRuntime_LIBRARIES} xtensor xtensor::optimize xtensor::use_xsimd)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

add_subdirectory(test)
