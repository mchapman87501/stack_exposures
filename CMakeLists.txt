
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
    GIT_TAG 102375523e845c5bdb34e03cae2cd3aba7247a89
)
FetchContent_MakeAvailable(arg_parse)

include(cmake_modules/CodeCoverage.cmake)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(STACK_EXP_SRC
    src/image_loader.cpp
    src/image_aligner.cpp
    src/image_info.cpp
    src/image_stacker.cpp
    src/str_util.cpp)

add_library(stack_exp STATIC ${STACK_EXP_SRC})
target_compile_features(stack_exp PUBLIC cxx_std_20)
target_include_directories(stack_exp
    PRIVATE include ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling#always-full-rpath
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

add_executable(stack_exposures src/main.cpp)
target_compile_features(stack_exposures PUBLIC cxx_std_20)
target_include_directories(stack_exposures
    PRIVATE include ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures
    PUBLIC stack_exp
    PRIVATE arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
install(
    TARGETS stack_exposures
    EXPORT stack_exposure_targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

find_package(Catch2 3 REQUIRED)
enable_testing()
include(CTest)
include(Catch)

add_subdirectory(tests)