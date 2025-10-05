cmake_minimum_required(VERSION 4.0)

project(taskproc
    VERSION 0.1.0
    DESCRIPTION "A CLI tool for processing task data"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for editors/tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wmost                      # Clang: enables many useful warnings
        -Wpessimizing-move
        -Wredundant-move
        -Wmove
        -Wrange-loop-analysis       # Warn about inefficient range-for loops
        -Wsuggest-override
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wunused
        -Woverloaded-virtual
    )
else()  # GCC
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wpessimizing-move
        -Wredundant-move
        -Wsuggest-override
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wunused
        -Woverloaded-virtual
    )
endif()

# Get fast-cpp-csv-parser (header-only)
include(FetchContent)

FetchContent_Declare(
    fastcsv
    GIT_REPOSITORY https://github.com/ben-strasser/fast-cpp-csv-parser.git
    GIT_TAG        master
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(fastcsv)

# Get nlohmann/json (header-only)
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(nlohmann_json)

# Create a library with business logic
add_library(taskproc_lib STATIC
    # CLI files
    cli/parser.hpp
    cli/parser.cpp

    # Core files
    core/task.hpp
    core/task.cpp
    core/view_action.hpp
    core/data_manager.hpp
    core/data_manager.cpp
    core/database.hpp
    core/database.cpp
    core/expr_parser.hpp
    core/expr_parser.cpp

    # IO files
    io/reader.hpp
    io/csv_reader.hpp
    io/csv_reader.cpp
    io/json_reader.hpp
    io/json_reader.cpp
    io/view_storage.hpp
    io/view_storage.cpp

)

target_include_directories(taskproc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Expose the downloaded fastcsv headers to taskproc_lib
target_include_directories(taskproc_lib SYSTEM PRIVATE
    ${fastcsv_SOURCE_DIR}
)

# Link nlohmann/json headers to taskproc_lib
target_link_libraries(taskproc_lib PRIVATE nlohmann_json::nlohmann_json)

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

    # Add tests directory ~/tests
    add_subdirectory(../tests tests)
endif()
