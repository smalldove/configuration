#!/bin/bash

# VoltageMonitor Project Build Script
# This script demonstrates the optimized CMakeLists.txt configuration

echo "=========================================="
echo "VoltageMonitor Project Build Script"
echo "=========================================="

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
    
    # Test the executable from project directory
    echo "Testing the executable from project directory:"
    if [ -f "project/VoltageMonitor" ]; then
        cd project && ./VoltageMonitor && cd ..
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
