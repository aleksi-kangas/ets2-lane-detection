cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR)
project(ets2ld_main CXX)

option(BUILD_TESTS "Build tests" ON)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.27: https://github.com/Kitware/CMake/blob/v3.27.9/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

add_subdirectory(capture)
add_subdirectory(ets2ld)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ets2ld")
target_link_libraries(${PROJECT_NAME} PUBLIC ets2ld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/imgui.ini"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_cuda.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_shared.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_tensorrt.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

install(TARGETS ${PROJECT_NAME}
        DESTINATION install
        RUNTIME_DEPENDENCIES
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll")

install(FILES
        "${CMAKE_SOURCE_DIR}/imgui.ini"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_cuda.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_shared.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_tensorrt.dll"
        DESTINATION install)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs"
        DESTINATION install
        FILES_MATCHING
        PATTERN "*.md"
        PATTERN "BUILD.md" EXCLUDE)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/models"
        DESTINATION install
        FILES_MATCHING PATTERN "*.md")

if (${BUILD_TESTS})
    enable_testing()
endif ()
