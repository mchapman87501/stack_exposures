
add_executable(test_image_info
    src/test_image_info.cpp
    ../src/image_info.cpp)
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(test_image_info
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_info PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_info)


add_executable(test_image_loader
    src/test_image_loader.cpp
    ../src/image_loader.cpp
    ../src/image_info.cpp)
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(test_image_loader
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_loader PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_loader)

# Define a coverage target:
include("../cmake_modules/CodeCoverage.cmake")
append_coverage_compiler_flags()
SET(COVERAGE_LCOV_EXCLUDES 
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}/tests"
    "{OpenCV_INCLUDE_DIRS}"
    "${LibRaw_INCLUDE_DIR}")
setup_target_for_coverage_lcov(NAME coverage_lcov
                                EXECUTABLE ctest
                                DEPENDENCIES test_image_info test_image_loader)