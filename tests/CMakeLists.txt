if(${CMAKE_BUILD_TYPE} STREQUAL "Profile")
    # https://gcovr.com/en/stable/cookbook.html#oos-cmake
    # This should work with both gcc and clang.
    set(CMAKE_C_FLAGS_PROFILE "-O0 --coverage")
    set(CMAKE_CXX_FLAGS_PROFILE "-O0 --coverage")
endif()

# Create a version of the library that's compiled with coverage flags.
string(REPLACE "src/" ";../src/" COV_SOURCES ${STACK_EXP_SRC})
add_library(stack_exp_cov STATIC ${COV_SOURCES})
target_compile_features(stack_exp_cov PUBLIC cxx_std_20)

target_include_directories(
    stack_exp_cov
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp_cov PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

add_executable(test_image_info src/test_image_info.cpp)
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(
    test_image_info
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(
    test_image_info
    PUBLIC stack_exp_cov
    PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_info PROPERTIES LABELS "Unit")

add_executable(test_image_loader src/test_image_loader.cpp)
set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data/")
target_compile_definitions(test_image_loader
    PUBLIC TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(
    test_image_loader
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(
    test_image_loader
    PUBLIC stack_exp_cov
    PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_loader PROPERTIES LABELS "Unit")

add_executable(test_image_stacker src/test_image_stacker.cpp)
set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data/")
target_compile_definitions(test_image_stacker
    PUBLIC TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_features(test_image_stacker PUBLIC cxx_std_20)
target_include_directories(
    test_image_stacker
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(
    test_image_stacker
    PUBLIC stack_exp_cov
    PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_stacker PROPERTIES LABELS "Unit")

add_executable(test_image_aligner src/test_image_aligner.cpp)
target_compile_definitions(test_image_aligner
    PUBLIC TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_features(test_image_aligner PUBLIC cxx_std_20)
target_include_directories(
    test_image_aligner
    PUBLIC ../include
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
    ${LibRaw_INCLUDE_DIR})
target_link_libraries(
    test_image_aligner
    PUBLIC stack_exp_cov
    PRIVATE Catch2::Catch2WithMain arg_parse::arg_parse ${OpenCV_LIBS}
    ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_aligner PROPERTIES LABELS "Unit")

# Integration test:
add_executable(stack_exposures_cov ../src/main.cpp)
target_compile_features(stack_exposures_cov PUBLIC cxx_std_20)
target_include_directories(
    stack_exposures_cov
    PUBLIC ../include
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
    ${LibRaw_INCLUDE_DIR})
target_link_libraries(
    stack_exposures_cov
    PUBLIC stack_exp_cov
    PRIVATE arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# =======================================================================
# Integration tests
# -----------------------------------------------------------------------
# TODO remove the output image at the end of the test:
set(pit_img "${TEST_DATA_DIR}/exif_extractor_missing_icc.jpg")
add_test(NAME positive_integration_test
    COMMAND stack_exposures_cov -o "pit_result.jpg" ${pit_img} ${pit_img}
    ${pit_img})
set_tests_properties(positive_integration_test
    PROPERTIES
    LABELS "Integration")

add_test(NAME understands_show_help_1 COMMAND stack_exposures_cov -h)
set_tests_properties(understands_show_help_1
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage:"
    LABELS "Integration")

add_test(NAME understands_show_help_2 COMMAND stack_exposures_cov --help)
set_tests_properties(understands_show_help_2
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage:"
    LABELS "Integration")

add_test(NAME positive_integration_test_no_align
    COMMAND stack_exposures_cov --no-align -o "pit_unaligned_result.jpg"
    ${pit_img} ${pit_img} ${pit_img} ${pit_img})

add_test(NAME invalid_output_format COMMAND stack_exposures_cov -o "ism.bogus"
    ${pit_img} ${pit_img})
set_tests_properties(
    invalid_output_format
    PROPERTIES
    WILL_FAIL true
    FAIL_REGULAR_EXPRESSION "not supported"
    LABELS "Integration")

add_test(NAME stack_exposures_no_args_given COMMAND stack_exposures_cov)
set_tests_properties(stack_exposures_no_args_given
    PROPERTIES
    WILL_FAIL true
    FAIL_REGULAR_EXPRESSION "Usage:"
    LABELS "Integration")

add_test(NAME stack_exposures_unknown_option COMMAND stack_exposures_cov --foo)
set_tests_properties(
    stack_exposures_unknown_option
    PROPERTIES
    WILL_FAIL true
    FAIL_REGULAR_EXPRESSION "Unknown option '--foo'"
    LABELS "Integration")

set(pit2_img "${TEST_DATA_DIR}/exif_extractor_missing_icc_2.jpg")
add_test(NAME stack_exposures_multiple_image_sizes
    COMMAND stack_exposures_cov -o "pit_mis.jpg" ${pit_img} ${pit2_img})
set_tests_properties(
    stack_exposures_multiple_image_sizes
    PROPERTIES
    WILL_FAIL true
    FAIL_REGULAR_EXPRESSION "Cannot process"
    LABELS "Integration")

foreach(suffix IN ITEMS jpg png tif tiff)
    set(test_name "stack_exposures_suffix_${suffix}")
    add_test(NAME ${test_name}
        COMMAND stack_exposures_cov -o "pit_suffix.${suffix}" ${pit_img}
        ${pit_img})
    set_tests_properties(${test_name}
        PROPERTIES
        LABELS "Integration OutputFormat")
endforeach()

# This is ridiculous.  It should produce black images but doesn't verify as
# much. It merely verifies that "-d <dark_image>" is valid usage.
add_test(NAME stack_exposures_darken
    COMMAND stack_exposures_cov -o "darken_.png" -d ${pit_img} ${pit_img}
    ${pit_img})
set_tests_properties(stack_exposures_darken
    PROPERTIES
    LABELS "Integration")

get_target_property(CATCH2_INCLUDE_DIRS Catch2::Catch2
    INTERFACE_INCLUDE_DIRECTORIES)
set(CATCH2_COV_EXC "")

foreach(entry IN ITEMS ${CATCH2_INCLUDE_DIRS})
    list(APPEND CATCH2_COV_EXC "${entry}/*")
endforeach()

# LCOV 2.0 appears to be the reason why I needed to change this.
# See discussion at
# https://bytemeta.vip/repo/linux-test-project/lcov/issues/220
set(REPORT_NAME coverage_report)

set(CLEAN_PROFILE_CMD
    lcov --directory ${CMAKE_BINARY_DIR} -b ${PROJECT_SOURCE_DIR}/src --zerocounters)

# Grrr: as of this writing, homebrew mac installs lcov 2.0.
# linux distros use lcov 1.x.
# 2.0 requires "--ignore-errors inconsistent,inconsistent" when running w.
# clang profiling output.  1.x does not understand those options.
set(LCOV_IGNORE_ARG "--ignore-errors=inconsistent,inconsistent")
execute_process(
    COMMAND lcov "--version"
    OUTPUT_VARIABLE LCOV_VERSION_OUT)

if(LCOV_VERSION_OUT MATCHES "LCOV version 1\.")
    set(LCOV_IGNORE_ARG "")
endif()

set(CAPTURE_PROFILE_CMD
    lcov --capture --directory ${CMAKE_CURRENT_BINARY_DIR}
    -b ${PROJECT_SOURCE_DIR}/src
    --no-external
    ${LCOV_IGNORE_ARG}
    --output-file coverage.info)

set(GEN_REPORT_CMD
    genhtml
    --demangle-cpp
    --rc derive_function_end_line=0
    --output-directory ${CMAKE_BINARY_DIR}/${REPORT_NAME}
    coverage.info
)

# LCOV 2.0 appears to be the reason why I needed to change this.
# See discussion at
# https://bytemeta.vip/repo/linux-test-project/lcov/issues/220
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/${REPORT_NAME}/index.html
    COMMAND ${CLEAN_PROFILE_CMD}
    COMMAND ctest
    COMMAND ${CAPTURE_PROFILE_CMD}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${REPORT_NAME}
    COMMAND ${GEN_REPORT_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS test_image_info test_image_loader test_image_stacker test_image_aligner stack_exposures_cov
)

add_custom_target(
    coverage_report
    DEPENDS ${CMAKE_BINARY_DIR}/${REPORT_NAME}/index.html
)
