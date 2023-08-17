cmake_minimum_required(VERSION 3.26...3.26 FATAL_ERROR)
project(ETS2_Lane_Detection CXX)

option(BUILD_TESTS "Build tests" ON)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

# Enable support for C++20 modules
# CMake 3.26: https://github.com/Kitware/CMake/blob/v3.26.4/Help/dev/experimental.rst
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

add_subdirectory(capture)
add_subdirectory(ets2ld)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ets2ld)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_cuda.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_shared.dll"
        "${VCPKG_INSTALLED_DIR}/x64-windows/bin/onnxruntime_providers_tensorrt.dll"
        DESTINATION install)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        DESTINATION install
        FILES_MATCHING
        PATTERN "*.md"
        PATTERN "BUILD.md" EXCLUDE)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/models"
        DESTINATION install
        FILES_MATCHING PATTERN "*.md")

if (${BUILD_TESTS})
    enable_testing()
endif ()
