#!/bin/sh
set -e -u


cd

cmake -Bbuild/release -S/source \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/source/build_artifacts/local
cd build/release
cmake --build . -j 4
cmake --install .

# Prove that we got a usable, installed executable:
(cd && /source/build_artifacts/local/bin/stack_exposures --help)