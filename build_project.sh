#!/bin/bash

# VoltageMonitor Project Build Script
# This script demonstrates the optimized CMakeLists.txt configuration

set -euo pipefail

echo "=========================================="
echo "VoltageMonitor Project Build Script"
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

LOCAL_DEPS_PREFIX="${SCRIPT_DIR}/third_party/prefix"
LOCAL_DEPS_LIB="${LOCAL_DEPS_PREFIX}/usr/lib/x86_64-linux-gnu"

# Fetch/extract deps into user-writable third_party when system -dev packages are missing (no sudo)
ensure_local_deps() {
    if [[ -f "${LOCAL_DEPS_PREFIX}/usr/include/yaml.h" \
       && -f "${LOCAL_DEPS_PREFIX}/usr/include/cjson/cJSON.h" \
       && -f "${LOCAL_DEPS_LIB}/libck.a" ]]; then
        echo "Local third_party deps already present."
        return 0
    fi

    echo "Preparing local deps under third_party/prefix (no sudo)..."
    mkdir -p "${LOCAL_DEPS_PREFIX}" /tmp/vm-depdebs
    (
        cd /tmp/vm-depdebs
        apt-get download libyaml-dev libcjson1 libcjson-dev libck0 libck-dev
        for deb in *.deb; do
            dpkg-deb -x "$deb" "${LOCAL_DEPS_PREFIX}"
        done
    )
}

ensure_local_deps
export LD_LIBRARY_PATH="${LOCAL_DEPS_LIB}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

# Clean any existing build artifacts
echo "Cleaning previous build artifacts..."
rm -rf build_about

# Configure the project with CMake (this will automatically create build_about directory)
echo "Configuring project with CMake..."
cmake -B build_about -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=1

# Build the project
echo "Building the project..."
cmake --build build_about

# Check if build was successful
if [ -f "build_about/bin/VoltageMonitor" ]; then
    echo "=========================================="
    echo "Build successful!"
    echo "Executable location: build_about/bin/VoltageMonitor"
    echo "Build artifacts location: build_about/"
    echo "=========================================="
    
    # Move executable to project directory
    echo "Moving executable to project directory..."
    cp build_about/bin/VoltageMonitor project/VoltageMonitor
    
    # Display build information
    echo "Build information:"
    cmake --build build_about --target build-info
    
    # Test the executable from project directory (main() loops forever; bound the run)
    echo "Testing the executable from project directory (timeout 5s)..."
    if [ -f "project/VoltageMonitor" ]; then
        if timeout 5s bash -c 'cd project && ./VoltageMonitor'; then
            echo "Executable exited within timeout (unexpected but OK)."
        else
            exit_code=$?
            # 124 = timeout killed the still-running process (expected)
            if [ "$exit_code" -eq 124 ]; then
                echo "Executable started successfully (stopped after 5s timeout)."
            else
                echo "Executable failed with exit code $exit_code"
                exit "$exit_code"
            fi
        fi
    else
        echo "Error: Executable not found in project directory"
        exit 1
    fi
else
    echo "=========================================="
    echo "Build failed!"
    echo "=========================================="
    exit 1
fi
