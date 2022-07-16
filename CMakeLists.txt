
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

include(FetchContent)
FetchContent_Declare(
    arg_parse
    GIT_REPOSITORY https://github.com/mchapman87501/arg_parse.git
    GIT_TAG 3b197a7f15fcf8799120926171d9bbc525c7b45c
)
FetchContent_MakeAvailable(arg_parse)

include(cmake_modules/CodeCoverage.cmake)

set(STACK_EXP_SRC
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp
    src/mean_image_stacker.cpp)

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
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures PUBLIC stack_exp PRIVATE arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

find_package(Catch2 3 REQUIRED)
enable_testing()
include(CTest)
include(Catch)

add_subdirectory(tests)