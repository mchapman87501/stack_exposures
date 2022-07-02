
add_executable(test_image_info src/test_image_info.cpp)
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(test_image_info
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_info
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_info)


add_executable(test_image_loader src/test_image_loader.cpp)
set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data/")
target_compile_definitions(test_image_loader PUBLIC TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(test_image_loader
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_loader
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_loader)

add_executable(test_image_aligner src/test_image_aligner.cpp)
target_compile_features(test_image_aligner PUBLIC cxx_std_20)
target_include_directories(test_image_aligner
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_aligner
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_aligner)

# Integration test:
add_executable(stack_exposures_cov ../src/main.cpp)
target_compile_features(stack_exposures_cov PUBLIC cxx_std_20)
target_include_directories(stack_exposures_cov
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures_cov PUBLIC stack_exp_cov PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# TODO remove the output image at the end of the test:
set(pit_img "${CMAKE_SOURCE_DIR}/tests/data/exif_extractor_missing_icc.jpg")
add_test(
    NAME positive_integration_test
    COMMAND stack_exposures_cov -o "pit_result.jpg" ${pit_img} ${pit_img} ${pit_img})
add_test(
    NAME understands_show_help_1
    COMMAND stack_exposures_cov -h)
add_test(
    NAME understands_show_help_2
    COMMAND stack_exposures_cov --help)
add_test(
    NAME positive_integration_test_no_align
    COMMAND stack_exposures_cov --no-align -o "pit_unaligned_result.jpg" ${pit_img} ${pit_img} ${pit_img})
# TODO show help when no args given
# TODO test unknown option
# TODO test input images with inconsistent sizes

append_coverage_compiler_flags()
setup_target_for_coverage_lcov(NAME coverage_report
    EXECUTABLE ctest
    DEPENDENCIES
        test_image_info test_image_loader test_image_aligner
        stack_exposures_cov
    EXCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src/*" "/opt/homebrew/*")