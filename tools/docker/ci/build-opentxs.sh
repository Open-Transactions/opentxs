#!/bin/bash

SRC="${1}"
COMPILER="${2}"
PRESET="${3}"
WORK="/home/output"

if [ ! -d "${SRC}" ]; then
    echo "Source tree missing. Mount opentxs source directory at ${SRC}"
    exit 1
fi

if [ ! -d "${WORK}" ]; then
    echo "Work directory missing. Mount build directory at ${WORK}"
    exit 1
fi

if [ "${COMPILER}" == "" ]; then
    echo "Compiler. Allowed values: gcc clang"
    exit 1
elif [ "${COMPILER}" == "gcc" ]; then
    export CMAKE_C_COMPILER="/usr/bin/gcc"
    export CMAKE_CXX_COMPILER="/usr/bin/g++"
elif [ "${COMPILER}" == "clang" ]; then
    export CMAKE_C_COMPILER="/usr/bin/clang"
    export CMAKE_CXX_COMPILER="/usr/bin/clang++"
else
    echo "Unknown compiler: ${COMPILER}"
    echo "Allowed values: gcc clang"
    exit 1
fi

set -e

rm -rf "${WORK}/"*
/usr/bin/cmake \
    -S "${SRC}" \
    -B "${WORK}" \
    --preset "${PRESET}" \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
    -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
/usr/bin/cmake \
    --build "${WORK}" \
    -- -k 0
