
cmake_policy(VERSION 3.23)
project(stack_exposures VERSION 1.0.0)

cmake_minimum_required(VERSION 3.23)

add_definitions("-std=c++20")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(stack_exposures
    src/main.cpp
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp)

target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(stack_exposures PUBLIC include ${CONAN_INCLUDE_DIRS})
target_link_libraries(stack_exposures ${CONAN_LIBS})