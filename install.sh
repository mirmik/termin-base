#!/bin/bash
set -e

BUILD_TYPE="Release"
PREFIX="/usr/local"

for arg in "$@"; do
    case "$arg" in
        --debug) BUILD_TYPE="Debug" ;;
        --prefix=*) PREFIX="${arg#*=}" ;;
        *) echo "Usage: $0 [--debug] [--prefix=/usr/local]"; exit 1 ;;
    esac
done

BUILD_DIR="build/${BUILD_TYPE}"

mkdir -p "${BUILD_DIR}"

cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_INSTALL_PREFIX="${PREFIX}"

cmake --build "${BUILD_DIR}" -j$(nproc)

sudo cmake --install "${BUILD_DIR}"

echo ""
echo "Installed termin_base (${BUILD_TYPE}) to ${PREFIX}"
