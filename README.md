# Overview

This is a CLI for stacking multiple image exposures.  It relies on opencv and libraw.

## Building

### On Host
To compile and run the stack_exposures executable:
```shell
mkdir -p build/release
cd build/release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../..
make
./stack_exposures --help
```

### Using Docker

```shell
sh ./docker_scripts/on_host/create_image.sh
sh ./docker_scripts/on_host/build_stack_exposures.sh
```

If the above steps complete successfully, you should find a `stack_exposures` executable
in `./build_artifacts`.

## Running Tests

### On Host

```shell
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make coverage_report
# If all tests pass:
open ./coverage_report/index.html
```

### Using Docker

```shell
sh ./docker_scripts/on_host/create_image.sh
sh ./docker_scripts/on_host/run_tests.sh
# If all tests pass:
open ./build_artifacts/coverage_report/index.html
```

## Formatting with clang-format

**NB:** This depends on `fd`, an alternative to `find`.

```shell
clang-format -i $(fd '.*\.(cpp|hpp)')
```

## Static Analysis with clang-tidy

If you have clang-tidy on your PATH:

```shell
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../..
clang-tidy ../../src/*.cpp ../../tests/src/*.cpp
```


## Future Work

Using something like exiv2, cluster images into multiple exposure bursts based on creation date; and stack each burst separately.