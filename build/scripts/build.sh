#!/bin/bash

set -e

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
JOBS=$(nproc)
ENABLE_SIMD="ON"
ENABLE_TESTING="ON"
ENABLE_OPENMP="OFF"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --no-simd)
            ENABLE_SIMD="OFF"
            shift
            ;;
        --no-tests)
            ENABLE_TESTING="OFF"
            shift
            ;;
        --openmp)
            ENABLE_OPENMP="ON"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug         Build in debug mode"
            echo "  --build-dir DIR Specify build directory (default: build)"
            echo "  --prefix DIR    Install prefix (default: /usr/local)"
            echo "  --jobs N        Number of parallel jobs (default: $(nproc))"
            echo "  --no-simd       Disable SIMD optimizations"
            echo "  --no-tests      Disable testing"
            echo "  --openmp        Enable OpenMP support"
            echo "  --help          Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "Building AnalyzeMFT C++"
echo "======================"
echo "Build type: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Install prefix: $INSTALL_PREFIX"
echo "Jobs: $JOBS"
echo "SIMD: $ENABLE_SIMD"
echo "Testing: $ENABLE_TESTING"
echo "OpenMP: $ENABLE_OPENMP"
echo

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DENABLE_SIMD="$ENABLE_SIMD" \
    -DENABLE_TESTING="$ENABLE_TESTING" \
    -DENABLE_OPENMP="$ENABLE_OPENMP"

# Build
cmake --build . --parallel "$JOBS"

# Run tests if enabled
if [ "$ENABLE_TESTING" = "ON" ]; then
    echo
    echo "Running tests..."
    ctest --parallel "$JOBS" --output-on-failure
fi

echo
echo "Build completed successfully!"
echo "Executable: $BUILD_DIR/analyzemft"
echo "Library: $BUILD_DIR/libanalyzemft.*"

if [ "$BUILD_TYPE" = "Release" ]; then
    echo
    echo "To install system-wide, run:"
    echo "  sudo cmake --install $BUILD_DIR"
fi