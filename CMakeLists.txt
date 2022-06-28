
cmake_policy(VERSION 3.23)
project(stack_exposures VERSION 1.0.0)

cmake_minimum_required(VERSION 3.23)

# How to determine, reliably, what variables are set by these find_package
# invocations?
find_package(OpenCV CONFIG REQUIRED PATHS)
find_package(LibRaw REQUIRED)

add_executable(stack_exposures
    src/main.cpp
    src/aligned_image_generator.cpp
    src/image_info.cpp
    src/image_stacker.cpp)

target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(
    stack_exposures PUBLIC
    include ${OpenCV_INCLUDE_DIR} ${LibRaw_INCLUDE_DIR})

# Darnit, what variables are defined by find_package(LibRaw)?
target_link_directories(stack_exposures PUBLIC ${OpenCV_LIB_DIRS} ${LibRaw_LIB_DIR})
target_link_libraries(
    stack_exposures PUBLIC ${OpenCV_LIBS} ${LibRaw_LIBRARY_RELEASE})