
cmake_policy(VERSION 3.22)
project(stack_exposures VERSION 1.0.0)

cmake_minimum_required(VERSION 3.22)

find_package(OpenCV REQUIRED)

# How did FindLibRaw.cmake come to be included as source?  See the README.md at
# https://github.com/LibRaw/LibRaw-cmake
# NOTE: Ignore the instructions on that page for how to modify CMakeLists.txt.
# I want to use the system-provided LibRaw, not to build it.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/extern/LibRaw-cmake/cmake/modules)
find_package(LibRaw REQUIRED)

include(cmake_modules/CodeCoverage.cmake)

set(STACK_EXP_SRC
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp)

add_library(stack_exp STATIC ${STACK_EXP_SRC})
target_compile_features(stack_exp PUBLIC cxx_std_20)
target_include_directories(stack_exp
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# Create a version of the library that's compiled with coverage flags from CodeCoverage.cmake
add_library(stack_exp_cov STATIC ${STACK_EXP_SRC})
target_compile_features(stack_exp_cov PUBLIC cxx_std_20)

# CURSES:  append_coverage_compiler_flags_to_target() fails because the multi-word value of CODE_COVERAGE_FLAGS is quoted.
string(REPLACE " " ";" COVERAGE_FLAGS_LIST ${COVERAGE_COMPILER_FLAGS})
target_compile_options(stack_exp_cov BEFORE PRIVATE ${COVERAGE_FLAGS_LIST})

target_include_directories(stack_exp_cov
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp_cov PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})



add_executable(stack_exposures src/main.cpp src/cmd_options.cpp)
target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(stack_exposures
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures PUBLIC stack_exp PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})


find_package(Catch2 3 REQUIRED)
enable_testing()
include(CTest)
include(Catch)

add_subdirectory(tests)