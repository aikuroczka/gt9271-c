#!/bin/bash

set -e

BUILD_DIR="build"
PROJECT_NAME="gt9271-c"

echo "-------------------------------------------------------"
echo " Performing Clean Verification Build: $PROJECT_NAME "
echo "-------------------------------------------------------"

if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found."
    exit 1
fi

echo "Cleaning previous artifacts..."
rm -rf $BUILD_DIR

mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo "Configuring project..."
cmake -S .. -B .

# Portable Core Detection
JOBS=1
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu)
fi

echo "Compiling source code with $JOBS jobs..."
cmake --build . -j $JOBS

echo "-------------------------------------------------------"
echo " Verification Build Completed Successfully!"
echo "-------------------------------------------------------"
