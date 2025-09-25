# Tests CMakeLists.txt

# Create test executable
add_executable(taskproc_tests
    test_parser.cpp
    test_data_manager.cpp
    test_csv_reader.cpp
    test_json_reader.cpp
)

# Link against Catch2
target_link_libraries(taskproc_tests PRIVATE
    taskproc_lib
    Catch2::Catch2WithMain
)

# Set include directories
target_include_directories(taskproc_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compiler flags for tests
target_compile_options(taskproc_tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Register tests with CTest
add_test(NAME unit_tests COMMAND taskproc_tests)

# Add custom test target for convenience
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS taskproc_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
