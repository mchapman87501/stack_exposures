#!/bin/sh
set -e -u

mkdir -p build_artifacts
docker run --rm -v${PWD}:/source \
    build_stack_exposures "/source/scripts/docker/in_container/test.sh"
