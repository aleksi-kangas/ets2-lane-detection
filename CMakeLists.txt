cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(ets2ld_main CXX)

MESSAGE(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
if (VCPKG_BUILD_TYPE)
    MESSAGE(STATUS "VCPKG build type: ${VCPKG_BUILD_TYPE}")
endif ()

option(BUILD_TESTS "Build tests" ON)

if (NOT MSVC)
    message(FATAL_ERROR "Only MSVC is supported")
endif ()

add_subdirectory(capture)
add_subdirectory(ets2ld)
add_subdirectory(ufld)

add_executable(${PROJECT_NAME} WIN32 main.cpp)
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
