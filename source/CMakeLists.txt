cmake_minimum_required(VERSION 4.0)

project(taskproc
    VERSION 0.1.0
    DESCRIPTION "A CLI tool for processing task data"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for editors/tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Create a library with business logic
add_library(taskproc_lib STATIC
    # CLI files
    cli/parser.hpp
    cli/parser.cpp

    # Core files
    core/task.hpp
    core/data_manager.hpp
    core/data_manager.cpp

    # IO files
    io/reader.hpp
    io/csv_reader.hpp
    io/csv_reader.cpp
    io/json_reader.hpp
    io/json_reader.cpp
)

target_include_directories(taskproc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Main executable
add_executable(taskproc
    main.cpp
)

target_link_libraries(taskproc PRIVATE taskproc_lib)
target_include_directories(taskproc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Testing configuration
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()

    # Find Catch2 (system installation)
    find_package(Catch2 3 REQUIRED)

    # Add tests subdirectory
    add_subdirectory(../tests tests)
endif()
