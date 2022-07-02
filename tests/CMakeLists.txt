
add_executable(test_image_info src/test_image_info.cpp)
target_compile_features(test_image_info PUBLIC cxx_std_20)
target_include_directories(test_image_info
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_info
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_info)


add_executable(test_image_loader src/test_image_loader.cpp)
target_compile_definitions(test_image_loader PUBLIC TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data/")
target_compile_features(test_image_loader PUBLIC cxx_std_20)
target_include_directories(test_image_loader
    PUBLIC ../include
    PRIVATE ${OpenCV_INCLUDE_DIRS} ${LibRaw_INCLUDE_DIR})
target_link_libraries(test_image_loader
    PUBLIC stack_exp_cov PRIVATE Catch2::Catch2WithMain ${OpenCV_LIBS} ${LibRaw_LIBRARIES})
catch_discover_tests(test_image_loader)

append_coverage_compiler_flags()
setup_target_for_coverage_lcov(NAME coverage_report
    EXECUTABLE ctest
    DEPENDENCIES test_image_info test_image_loader
    EXCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src/*" "/opt/homebrew/*")