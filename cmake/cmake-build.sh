#!/usr/bin/env bash

set -e

#Set Script Name variable
SCRIPT=$(basename "${BASH_SOURCE[0]}")

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

OUTPUT_DIR="${BUILD_TYPE,,}"
mkdir -p "${OUTPUT_DIR}"
cd "${OUTPUT_DIR}" || exit 1
cmake -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -D CMAKE_MAKE_PROGRAM=ninja \
      -G Ninja \
      ${CMAKE_FLAGS} \
      ..

ninja -v
#    ninja package
