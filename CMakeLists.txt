
cmake_policy(VERSION 3.22)
project(stack_exposures VERSION 1.0.0)

cmake_minimum_required(VERSION 3.22)

# On linux, conan seems intent on downloading the entire known universe.
#add_definitions("-std=c++20")
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()

find_package(OpenCV REQUIRED CONFIG)
find_package(LibRaw REQUIRED)

add_executable(stack_exposures
    src/main.cpp
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp)

target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(stack_exposures PUBLIC include PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures ${OpenCV_LIBS} ${LibRaw_LIBRARIES})