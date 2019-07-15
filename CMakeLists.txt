cmake_minimum_required(VERSION 3.12)
include(GenerateExportHeader) 

project(htp VERSION 0.1)

# Putting binaries in the output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/bin)

# General poject settings
option(HTP_BUILD_SHARED_LIB "Build shared library" OFF)
add_compile_options("/EHsc") # Using map and vector among other things.

# -----------------------------------------------------------------------------
# Building external dependencies
# -----------------------------------------------------------------------------
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/external/Zydis/.git")
    message(FATAL_ERROR "Zydis submodle not initialized, 
            please run git submodule update --init --recursive")
endif()
add_subdirectory("external/Zydis")

#utils directory
set(UTILS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/utils")

if(HTP_BUILD_SHARED_LIB)
    add_library("htp" SHARED)
else()
    add_library("htp" STATIC)
endif()

# -----------------------------------------------------------------------------
# Generating configuration macros
# -----------------------------------------------------------------------------
generate_export_header("htp" BASE_NAME "HTP" 
                             EXPORT_FILE_NAME "htp_export_config.h")
target_include_directories("htp"
    PUBLIC "include" ${PROJECT_BINARY_DIR}
    PRIVATE "utils")
# -----------------------------------------------------------------------------
# Generating core libraries
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")

# -----------------------------------------------------------------------------
# Generating tests configuration
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")