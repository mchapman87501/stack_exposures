#!/bin/sh
set -e -u

# Run from the repository root directory.
cd $(dirname "$0")/../../../

mkdir -p build_artifacts
docker run --rm -v${PWD}:/source build_stack_exposures
