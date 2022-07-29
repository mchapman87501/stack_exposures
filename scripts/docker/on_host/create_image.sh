#!/bin/sh
set -e -u

# Run from the repository root directory.
cd $(dirname "$0")/../../../

docker build -t build_stack_exposures .