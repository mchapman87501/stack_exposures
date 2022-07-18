#!/bin/sh
set -e -u


cd ${HOME}
echo "HOME is ${PWD}"
mkdir -p build/release
cd build/release

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/source/build_artifacts/local \
      /source
cmake --build .
cmake --install .

# Prove that we got a usable, installed executable:
(cd && /source/build_artifacts/local/bin/stack_exposures --help)