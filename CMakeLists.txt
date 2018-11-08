cmake_minimum_required(VERSION 3.12)
include(GenerateExportHeader) 

project(htp VERSION 0.1)

# Putting binaries in the output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/bin)

# General poject settings
option(HTP_BUILD_SHARED_LIB "Build shared library" OFF)

# -----------------------------------------------------------------------------
# Building external dependencies
# -----------------------------------------------------------------------------
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/external/Zydis/.git")
    message(FATAL_ERROR "Zydis submodle not initialized, 
            please run git submodule update --init --recursive")
endif()
add_subdirectory("external/Zydis")

# Global include directory, for htp.h
include_directories("include")

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
    PRIVATE "src")
# -----------------------------------------------------------------------------
# Generating core libraries
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/agent/CMakeLists.txt")

# -----------------------------------------------------------------------------
# Generating tests configuration
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")