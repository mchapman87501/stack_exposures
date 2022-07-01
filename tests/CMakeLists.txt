
# I can't figure out how to get the coverage_lcov target
# to include coverage information for all of my library
# source files, except by listing them explicitly as test
# source files.  Unfortunately, that also means the tests
# are not testing w. the as-built library...
add_executable(test_image_info
    src/test_image_info.cpp
    ${STACK_EXP_SRC})
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(test_image_info
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_info
    PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_info)


add_executable(test_image_loader
    src/test_image_loader.cpp
    ${STACK_EXP_SRC})
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(test_image_loader
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_loader PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_loader)

# Define a coverage target:
include("../cmake_modules/CodeCoverage.cmake")
append_coverage_compiler_flags()
setup_target_for_coverage_lcov(NAME coverage_lcov
    EXECUTABLE ctest
    DEPENDENCIES test_image_info test_image_loader
    EXCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src/*" "/opt/homebrew/*")