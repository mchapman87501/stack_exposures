
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

# TODO Figure out how to add a name prefix "stack_exposures::stack_exposures" w.o. defining a package.
set(STACK_EXP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_aligner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_stacker.cpp)

add_library(stack_exp STATIC ${STACK_EXP_SRC})
target_compile_features(stack_exp PUBLIC cxx_std_20)
target_include_directories(stack_exp
    PUBLIC include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})


add_executable(stack_exposures src/main.cpp)

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