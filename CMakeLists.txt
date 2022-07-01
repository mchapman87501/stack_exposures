
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

find_package(Catch2 3 REQUIRED)

add_executable(stack_exposures
    src/main.cpp
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp)

target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(stack_exposures
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# Tests
add_executable(test_image_info
    tests/src/test_image_info.cpp
    src/image_info.cpp)
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(test_image_info
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_info PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

add_executable(test_image_loader
    tests/src/test_image_loader.cpp
    src/image_loader.cpp
    src/image_info.cpp)
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(test_image_loader
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_loader PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

include(CTest)
include(Catch)
catch_discover_tests(test_image_loader)