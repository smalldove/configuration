# =============================================================================
# VoltageMonitor Project - Main CMakeLists.txt
# =============================================================================
# 
# Features:
# - All build artifacts generated in build_about/ directory
# - Executable output in build_about/bin/ directory
# - Modular source file discovery
# - Comprehensive compiler options for Debug/Release builds
# - External dependency management (pthread, math)
# - Custom targets for build management
# =============================================================================

cmake_minimum_required(VERSION 3.10)
project(VoltageMonitor 
    VERSION 1.0.0
    DESCRIPTION "Voltage Monitor Application"
    LANGUAGES C
)

# =============================================================================
# Project Configuration
# =============================================================================

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "No build type specified, defaulting to ${CMAKE_BUILD_TYPE}")
endif()

# Set C standard to C11 with strict requirements
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# Build Directory Configuration
# =============================================================================

# Set all build-related directories to build_about
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build_about)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Force out-of-source build to prevent CMake from generating files in source directory
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Create build directories if they don't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# Set cache variables to ensure build directory is used
set(CMAKE_CACHE_DIR ${CMAKE_BINARY_DIR}/CMakeCache CACHE PATH "CMake cache directory")
set(CMAKE_FILES_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles CACHE PATH "CMake files directory")

# Print directory configuration
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Executable output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "CMake cache directory: ${CMAKE_CACHE_DIR}")
message(STATUS "CMake files directory: ${CMAKE_FILES_DIRECTORY}")

# =============================================================================
# Custom Build Commands
# =============================================================================

# Custom command to ensure build_about directory exists before any build
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/.build_ready
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/.build_ready
    COMMENT "Ensuring build_about directory is ready"
    VERBATIM
)

# =============================================================================
# Source Files Configuration
# =============================================================================

# Define source directories (modular structure)
set(SOURCE_DIRECTORIES
    src/main      # Main application entry point
    src/app       # Application logic and function registry
    src/mode_lib  # Mode-specific function implementations
    src/lib       # Library modules (balanced tree, etc.)
)

# Define include directories
set(INCLUDE_DIRECTORIES
    src/include   # Header files
)

# Collect all source files with validation
set(ALL_SOURCES)
foreach(SOURCE_DIR ${SOURCE_DIRECTORIES})
    file(GLOB_RECURSE SOURCES_${SOURCE_DIR} "${SOURCE_DIR}/*.c")
    if(SOURCES_${SOURCE_DIR})
        list(APPEND ALL_SOURCES ${SOURCES_${SOURCE_DIR}})
        message(STATUS "Found ${SOURCE_DIR} sources: ${SOURCES_${SOURCE_DIR}}")
    else()
        message(WARNING "No source files found in ${SOURCE_DIR}")
    endif()
endforeach()

# Validate that we have source files
if(NOT ALL_SOURCES)
    message(FATAL_ERROR "No source files found in specified directories!")
endif()

# =============================================================================
# Target Configuration
# =============================================================================

# Create the main executable
add_executable(VoltageMonitor ${ALL_SOURCES})

# Set output directory properties
set_target_properties(VoltageMonitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

# =============================================================================
# Include Directories
# =============================================================================

# Add include directories
foreach(INCLUDE_DIR ${INCLUDE_DIRECTORIES})
    target_include_directories(VoltageMonitor PRIVATE ${INCLUDE_DIR})
endforeach()

# =============================================================================
# Compiler Options
# =============================================================================

# Common compiler flags (applied to all builds)
set(COMMON_COMPILER_FLAGS
    -Wall                    # Enable all warnings
    -Wextra                  # Enable extra warnings
    -Wpedantic               # Strict ISO C compliance
    -Wformat=2               # Strict format string checking
    -Wconversion             # Warn about implicit conversions
    -Wsign-conversion        # Warn about sign conversions
    -Wshadow                 # Warn about shadowed variables
    -Wunused-parameter       # Warn about unused parameters
)

# Debug build options
set(DEBUG_COMPILER_FLAGS
    -g                       # Debug symbols
    -O0                      # No optimization
    -DDEBUG                  # Debug mode definition
    -fsanitize=address       # Address sanitizer
    -fsanitize=undefined     # Undefined behavior sanitizer
)

# Release build options
set(RELEASE_COMPILER_FLAGS
    -O2                      # Optimize for performance
    -DNDEBUG                 # No debug mode
    -Werror                  # Treat warnings as errors
    -flto                    # Link-time optimization
)

# Set compiler options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build with sanitizers")
    target_compile_options(VoltageMonitor PRIVATE 
        ${COMMON_COMPILER_FLAGS} 
        ${DEBUG_COMPILER_FLAGS}
    )
    target_link_options(VoltageMonitor PRIVATE 
        -fsanitize=address 
        -fsanitize=undefined
    )
else()
    message(STATUS "Configuring Release build with optimizations")
    target_compile_options(VoltageMonitor PRIVATE 
        ${COMMON_COMPILER_FLAGS} 
        ${RELEASE_COMPILER_FLAGS}
    )
    target_link_options(VoltageMonitor PRIVATE -flto)
endif()

# Platform-specific options
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_compile_options(VoltageMonitor PRIVATE -D_POSIX_C_SOURCE=200809L)
    message(STATUS "Configuring for Linux platform")
endif()

# =============================================================================
# External Dependencies
# =============================================================================

# pthread library (required for threading)
find_package(Threads REQUIRED)
if(Threads_FOUND)
    target_link_libraries(VoltageMonitor PRIVATE Threads::Threads)
    target_compile_definitions(VoltageMonitor PRIVATE HAVE_PTHREAD=1)
    message(STATUS "Found pthread library: YES")
else()
    message(FATAL_ERROR "pthread library not found - required for this project")
endif()

# Math library (optional - for mathematical functions)
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(VoltageMonitor PRIVATE ${MATH_LIBRARY})
    message(STATUS "Found math library: YES")
else()
    message(STATUS "Math library not found - continuing without it")
endif()

# YAML library (required for YAML parsing)
find_library(YAML_LIBRARY yaml)
if(YAML_LIBRARY)
    target_link_libraries(VoltageMonitor PRIVATE ${YAML_LIBRARY})
    target_compile_definitions(VoltageMonitor PRIVATE HAVE_YAML=1)
    message(STATUS "Found YAML library: YES")
else()
    message(FATAL_ERROR "YAML library not found - required for YAML parsing")
endif()

# cJSON library (required for JSON parsing)
find_library(CJSON_LIBRARY cjson)
if(CJSON_LIBRARY)
    target_link_libraries(VoltageMonitor PRIVATE ${CJSON_LIBRARY})
    target_compile_definitions(VoltageMonitor PRIVATE HAVE_CJSON=1)
    message(STATUS "Found cJSON library: YES")
else()
    message(FATAL_ERROR "cJSON library not found - required for JSON parsing")
endif()

# Concurrency Kit (ck) library (required for high-performance queues)
set(CK_LIBRARY_PATH /home/xcvbnm/ck-0.7.2/src/libck.a)
set(CK_INCLUDE_PATH /home/xcvbnm/ck-0.7.2/include)

if(EXISTS ${CK_LIBRARY_PATH} AND EXISTS ${CK_INCLUDE_PATH})
    target_link_libraries(VoltageMonitor PRIVATE ${CK_LIBRARY_PATH})
    target_include_directories(VoltageMonitor PRIVATE ${CK_INCLUDE_PATH})
    target_compile_definitions(VoltageMonitor PRIVATE HAVE_CK=1)
    message(STATUS "Found Concurrency Kit (ck) library: YES")
    message(STATUS "  Library: ${CK_LIBRARY_PATH}")
    message(STATUS "  Include: ${CK_INCLUDE_PATH}")
else()
    message(WARNING "Concurrency Kit (ck) library not found at ${CK_LIBRARY_PATH}")
    message(WARNING "Continuing without ck library support")
endif()

# =============================================================================
# Project Information
# =============================================================================

# Print detailed project information
message(STATUS "==========================================")
message(STATUS "VoltageMonitor Project Configuration")
message(STATUS "==========================================")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Runtime Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Library Output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "Archive Output: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "Source Files: ${ALL_SOURCES}")
message(STATUS "Include Directories: ${INCLUDE_DIRECTORIES}")
message(STATUS "==========================================")

# =============================================================================
# Custom Targets
# =============================================================================

# Clean build directory (removes all build artifacts)
add_custom_target(clean-build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VoltageMonitor
    COMMENT "Cleaning build directories and executable"
)

# Build info target (displays current configuration)
add_custom_target(build-info
    COMMAND ${CMAKE_COMMAND} -E echo "Build Information:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Project: ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Version: ${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Build Type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Build Directory: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Number of Source Files: ${CMAKE_ARGC}"
    COMMENT "Displaying build information"
)

# Test target (runs the executable)
add_custom_target(test-run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VoltageMonitor
    DEPENDS VoltageMonitor
    COMMENT "Running the VoltageMonitor executable"
)

# =============================================================================
# Installation (Optional)
# =============================================================================

# Install target
install(TARGETS VoltageMonitor
    DESTINATION bin
)
