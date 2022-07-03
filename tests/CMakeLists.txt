append_coverage_compiler_flags()

add_executable(test_cmd_options
    src/test_cmd_options.cpp ../src/cmd_options.cpp)
target_compile_features(test_cmd_options PUBLIC cxx_std_20)
target_include_directories(test_cmd_options
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_cmd_options
    PUBLIC PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_cmd_options)

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
target_compile_definitions(test_image_aligner PUBLIC TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_features(test_image_aligner PUBLIC cxx_std_20)
target_include_directories(test_image_aligner
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_aligner
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_aligner)

# Integration test:
add_executable(stack_exposures_cov ../src/main.cpp ../src/cmd_options.cpp)
target_compile_features(stack_exposures_cov PUBLIC cxx_std_20)
target_include_directories(stack_exposures_cov
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures_cov PUBLIC stack_exp_cov PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})


#=======================================================================
# Catch2-based tests
# Catch2 seems to offer no easy way to examine stderr; hence adding
# this instead of using catch_discover_tests alone.
#-----------------------------------------------------------------------
add_test(NAME test_cmd_options_stderr COMMAND test_cmd_options)
set_tests_properties(test_cmd_options_stderr
    PROPERTIES
        PASS_REGULAR_EXPRESSION "No output path")

#=======================================================================
# Integration tests
#-----------------------------------------------------------------------
# TODO remove the output image at the end of the test:
set(pit_img "${TEST_DATA_DIR}/exif_extractor_missing_icc.jpg")
add_test(
    NAME positive_integration_test
    COMMAND stack_exposures_cov -o "pit_result.jpg" ${pit_img} ${pit_img} ${pit_img})

add_test(
    NAME understands_show_help_1
    COMMAND stack_exposures_cov -h)
set_tests_properties(
    understands_show_help_1
    PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:")

add_test(
    NAME understands_show_help_2
    COMMAND stack_exposures_cov --help)
set_tests_properties(
    understands_show_help_2
    PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:")

add_test(
    NAME positive_integration_test_no_align
    COMMAND stack_exposures_cov --no-align -o "pit_unaligned_result.jpg" ${pit_img} ${pit_img} ${pit_img})

add_test(
    NAME stack_exposures_no_args_given
    COMMAND stack_exposures_cov)
set_tests_properties(
    stack_exposures_no_args_given
    PROPERTIES
        WILL_FAIL true
        FAIL_REGULAR_EXPRESSION "Usage:")

add_test(
    NAME stack_exposures_unknown_option
    COMMAND stack_exposures_cov --foo)
set_tests_properties(
    stack_exposures_unknown_option
    PROPERTIES
        WILL_FAIL true
        FAIL_REGULAR_EXPRESSION "Unknown option '--foo'")

set(pit2_img "${TEST_DATA_DIR}/exif_extractor_missing_icc_2.jpg")
add_test(
    NAME stack_exposures_multiple_image_sizes
    COMMAND stack_exposures_cov -o "pit_mis.jpg" ${pit_img} ${pit2_img})
set_tests_properties(
    stack_exposures_multiple_image_sizes
    PROPERTIES
        WILL_FAIL true
        FAIL_REGULAR_EXPRESSION "Cannot process")

setup_target_for_coverage_lcov(NAME coverage_report
    EXECUTABLE ctest
    DEPENDENCIES
        test_cmd_options
        test_image_info test_image_loader test_image_aligner
        stack_exposures_cov
    EXCLUDE
        "include/*"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*" "/opt/homebrew/*" "/Applications/*"
        "/usr/*")