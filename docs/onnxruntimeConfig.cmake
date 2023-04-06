# Custom cmake config file by jcarius to enable find_package(OnnxRuntime) without modifying LIBRARY_PATH and LD_LIBRARY_PATH
#
# This will define the following variables:
#   OnnxRuntime_FOUND        -- True if the system has the OnnxRuntime library
#   OnnxRuntime_INCLUDE_DIRS -- The include directories for OnnxRuntime
#   OnnxRuntime_LIBRARIES    -- Libraries to link against
#   OnnxRuntime_CXX_FLAGS    -- Additional (required) compiler flags

include(FindPackageHandleStandardArgs)

# Assume we are in <install-prefix>/share/cmake/onnxruntime/onnxruntimeConfig.cmake
get_filename_component(CMAKE_CURRENT_LIST_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(OnnxRuntime_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

set(OnnxRuntime_INCLUDE_DIRS ${OnnxRuntime_INSTALL_PREFIX}/include)
set(OnnxRuntime_LIBRARIES OnnxRuntime)
set(OnnxRuntime_CXX_FLAGS "") # no flags needed


find_library(OnnxRuntime_LIBRARY OnnxRuntime
        PATHS "${OnnxRuntime_INSTALL_PREFIX}/lib"
        )

add_library(OnnxRuntime SHARED IMPORTED)
set_property(TARGET OnnxRuntime PROPERTY IMPORTED_IMPLIB "${OnnxRuntime_LIBRARY}")
set_property(TARGET OnnxRuntime PROPERTY IMPORTED_LOCATION "${OnnxRuntime_LIBRARY}")
set_property(TARGET OnnxRuntime PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${OnnxRuntime_INCLUDE_DIRS}")
set_property(TARGET OnnxRuntime PROPERTY INTERFACE_COMPILE_OPTIONS "${OnnxRuntime_CXX_FLAGS}")

find_package_handle_standard_args(OnnxRuntime DEFAULT_MSG OnnxRuntime_LIBRARY OnnxRuntime_INCLUDE_DIRS)
