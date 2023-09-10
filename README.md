# Overview

This is a CLI for stacking multiple image exposures. It relies on [OpenCV](https://docs.opencv.org/4.5.5/) and [LibRaw](https://www.libraw.org/docs/API-CXX.html).

![Github Docker CI](https://github.com/mchapman87501/stack_exposures/actions/workflows/docker-image.yml/badge.svg)

## Building

### On Host

#### Dependencies

In order to build and test on macOS or linux you'll need the following (at least):

- [CMake](https://www.cmake.org)
- A C++ compiler ;)
- [Catch2](https://github.com/catchorg/Catch2)
- [lcov](https://github.com/linux-test-project/lcov.git)
- [OpenCV](https://docs.opencv.org/4.5.5/)
- [LibRaw](https://www.libraw.org/docs/API-CXX.html)

#### Compile and Run

To compile and run the `stack_exposures` executable:

```shell
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build/release -S .
cmake --build build/release
./build/release/stack_exposures --help
```

### Using Docker

```shell
sh ./scripts/docker/on_host/create_image.sh
sh ./scripts/docker/on_host/build_stack_exposures.sh
```

If steps above succeed, you can find a `stack_exposures`
executable in `build_artifacts/local/bin`.

## Running Tests

Here's how to run all unit tests and generate a coverage report.

### On Host

```shell
cmake -DCMAKE_BUILD_TYPE=Profile -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build/profile -S .
cmake --build build/profile --target coverage_report
```

If the steps above succeed, you can find a coverage report in `build/profile/coverage_report/index.html`.

### Using Docker

```shell
sh ./scripts/docker/on_host/create_image.sh
sh ./scripts/docker/on_host/run_tests.sh
```

If the steps above succeed, you can find a coverage report in
`build_artifacts/coverage_report/index.html`.

## Installing

It's expected that `stack_exposures` will be built from source, and run from where it's built. That said, you can install it (with some pain) as outlined below.

### On Host

You can install using `cmake --install .`. In order for the installed executable to find required libraries at runtime, you'll need to specify the installation directory at configuration time.

```shell
INSTALL_DIR=/where/to/install
cmake -Bbuild/release -S. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
cmake --build build/release
cmake --install build/release
${INSTALL_DIR}/bin/stack_exposures --help
```

### Using Docker

After building using the "Compile and Run" instructions for docker, as outlined above, you can copy the `.../bin/stack_exposures` and `.../lib/libparg_parse.so*` artifacts from the `build_artifacts` directory to wherever you want to install them. In order to use the executable you'll need to point the `LD_LIBRARY_PATH` environment variable to wherever you installed the `libarg_parse.so*` libraries.

## Formatting with clang-format

If you have both [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and [fd](https://github.com/sharkdp/fd.git) (an alternative to `find`) on your PATH:

```shell
clang-format -i $(fd '.*\.(cpp|hpp)')
```

## Static Analysis with clang-tidy

If you have [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) on your PATH:

```shell
cmake -Bbuild/release -S. -DCMAKE_BUILD_TYPE=Release
clang-tidy --checks="modernize-*,-modernize-use-trailing-return-type" --header-filter='stack_exposures/include/' -p build/release  src/*.cpp tests/src/*.cpp
```

## Future Work

Using something like [exiv2](https://exiv2.org), cluster images into multiple exposure bursts based on creation date; and stack each burst separately.
