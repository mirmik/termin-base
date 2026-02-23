#!/bin/bash
set -e

BUILD_TYPE="Release"
CLEAN=0
INSTALL_DIR="$(pwd)/install"

for arg in "$@"; do
    case "$arg" in
        --debug) BUILD_TYPE="Debug" ;;
        --release) BUILD_TYPE="Release" ;;
        --clean) CLEAN=1 ;;
        --install-dir=*) INSTALL_DIR="${arg#*=}" ;;
        *) echo "Unknown option: $arg"; exit 1 ;;
    esac
done

BUILD_DIR="build/${BUILD_TYPE}"

if [ "$CLEAN" -eq 1 ]; then
    echo "Cleaning build directory: ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
    rm -rf "${INSTALL_DIR}"
fi

mkdir -p "${BUILD_DIR}"

cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"

cmake --build "${BUILD_DIR}" -j$(nproc)

cmake --install "${BUILD_DIR}"

echo ""
echo "Build complete: ${BUILD_TYPE}"
echo "Installed to: ${INSTALL_DIR}"
