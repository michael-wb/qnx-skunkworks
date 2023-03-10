#!/usr/bin/env bash

set -e

#Set Script Name variable
SCRIPT=$(basename "${BASH_SOURCE[0]}")
CMAKE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)

function usage {
    echo "Usage: ${SCRIPT} -t <build_type> [-f <cmake_flags>]"
    echo ""
    echo "Arguments:"
    echo "   build_type=<Release|Debug>"
    exit 1;
}

# Parse the options
while getopts ":o:a:t:v:f:" opt; do
    case "${opt}" in
        t)
            BUILD_TYPE=${OPTARG}
            [ "${BUILD_TYPE}" == "Debug" ] ||
            [ "${BUILD_TYPE}" == "MinSizeDebug" ] ||
            [ "${BUILD_TYPE}" == "Release" ] || usage
            ;;
        f) CMAKE_FLAGS=${OPTARG};;
        *) usage;;
    esac
done

shift $((OPTIND-1))

# Check for obligatory fields
if [[ -z "${BUILD_TYPE}" ]]; then
    echo "ERROR: option -t is missing";
    usage
fi

source "${CMAKE_DIR}/qnx-env.sh"

OUTPUT_DIR="build-qnx-${QNX_ARCH}-${BUILD_TYPE}"
mkdir -p "${OUTPUT_DIR}"
cd "${OUTPUT_DIR}" || exit 1
cmake -D CMAKE_SYSTEM_NAME=QNX \
      -D QNX_BASE="${QNX_BASE}" \
      -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -D CMAKE_TOOLCHAIN_FILE="./cmake/qnx.toolchain.cmake" \
      -D CMAKE_MAKE_PROGRAM=ninja \
      -D CMAKE_BUILD_WITH_INSTALL_RPATH=On \
      -G Ninja \
      ${CMAKE_FLAGS} \
      ..

ninja -v
#    ninja package
