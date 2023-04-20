cmake_minimum_required(VERSION 3.22)
project(ETS2_Lane_Detection)

# Copy the OnnxRuntime cmake files to the vcpkg installed directory
# These are needed to make the find_package(OnnxRuntime) work
file(COPY
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/onnxruntimeConfig.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/onnxruntimeVersion.cmake"
        DESTINATION "${VCPKG_INSTALLED_DIR}/x64-windows/share/onnxruntime-gpu")

option(BUILD_TESTS "Build tests" ON)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

find_package(OpenCV REQUIRED)

add_subdirectory(dx11)
add_subdirectory(ets2ld)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE dx11 ets2ld ufld ${OpenCV_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Copy the OnnxRuntime provider DLLs which are lazy loaded
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_cuda.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_shared.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_tensorrt.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

if (${BUILD_TESTS})
    enable_testing()
endif ()
