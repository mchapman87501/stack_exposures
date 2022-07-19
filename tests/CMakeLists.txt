if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    append_coverage_compiler_flags()
endif()

# Create a version of the library that's compiled with coverage flags from CodeCoverage.cmake
string(REPLACE "src/" ";../src/" COV_SOURCES ${STACK_EXP_SRC})
add_library(stack_exp_cov STATIC ${COV_SOURCES})
target_compile_features(stack_exp_cov PUBLIC cxx_std_20)

target_include_directories(stack_exp_cov
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exp_cov PRIVATE ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

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
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_aligner
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_aligner)

add_executable(test_image_stacker src/test_image_stacker.cpp)
target_compile_features(test_image_stacker PUBLIC cxx_std_20)
target_include_directories(test_image_stacker
    PUBLIC ../include
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_stacker
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_stacker)

add_executable(test_mean_image_stacker src/test_mean_image_stacker.cpp)
target_compile_features(test_mean_image_stacker PUBLIC cxx_std_20)
target_include_directories(test_mean_image_stacker
    PUBLIC ../include
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_mean_image_stacker
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_mean_image_stacker)

# Integration test:
add_executable(stack_exposures_cov ../src/main.cpp)
target_compile_features(stack_exposures_cov PUBLIC cxx_std_20)
target_include_directories(stack_exposures_cov
    PUBLIC ../include
    PRIVATE ${arg_parse_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(stack_exposures_cov PUBLIC stack_exp_cov PRIVATE arg_parse::arg_parse ${OpenCV_LIBS} ${LibRaw_LIBRARIES})

# =======================================================================
# Integration tests
# -----------------------------------------------------------------------
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
    NAME positive_integration_test_method_m
    COMMAND stack_exposures_cov -o "pit_mm.png" --stacking-method m ${pit_img} ${pit_img})

add_test(
    NAME positive_integration_test_method_s
    COMMAND stack_exposures_cov -o "pit_ms.png" -s s ${pit_img} ${pit_img})

add_test(
    NAME invalid_stacking_method
    COMMAND stack_exposures_cov -o "ism.png" --stacking-method= ${pit_img} ${pit_img})
set_tests_properties(
    invalid_stacking_method
    PROPERTIES
    WILL_FAIL true
    FAIL_REGULAR_EXPRESSION "STACKING_METHOD")

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

foreach(stacking_method m s)
    foreach(suffix IN ITEMS jpg png tif tiff)
        set(test_name "stack_exposures_suffix_${stacking_method}_${suffix}")
        message(STATUS "Test name is ${test_name}")
        add_test(
            NAME ${test_name}
            COMMAND
            stack_exposures_cov --stacking-method=${stacking_method}
            -o "pit_suffix.${suffix}" ${pit_img} ${pit_img}
        )
    endforeach()
endforeach()

# This is ridiculous.  It should produce black images but doesn't verify as much.
# It merely verifies that "-d <dark_image>" is valid usage.
foreach (stacking_method m s)
    add_test(
        NAME stack_exposures_darken_${stacking_method}
        COMMAND
            stack_exposures_cov
            -s ${stacking_method}
            -o "darken_${stacking_method}.png"
            -d ${pit_img}
            ${pit_img} ${pit_img})
endforeach()

setup_target_for_coverage_lcov(NAME coverage_report
    EXECUTABLE ctest
    DEPENDENCIES
    test_image_info test_image_loader test_image_aligner
    test_image_stacker test_mean_image_stacker
    stack_exposures_cov
    EXCLUDE
    "include/*"
    "build/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*" "/opt/homebrew/*" "/Applications/*"
    "/usr/*")

# *nix only:
add_custom_command(TARGET coverage_report
    PRE_BUILD
    COMMAND find ${CMAKE_BINARY_DIR} -name '*.gcda' -exec rm "{}" "';'"
)
