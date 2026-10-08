cmake_minimum_required(VERSION 3.25)
project(cpp_orderbook LANGUAGES CXX)

# --- Language standard -------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++20, not -std=gnu++20

# Writes build/compile_commands.json so clangd / IDEs understand the build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ORDERBOOK_SANITIZE "Build with AddressSanitizer + UBSan" OFF)

# --- Shared compile settings -------------------------------------------------
# An INTERFACE library holds no code, only settings. Targets that link it
# inherit its flags (similar to a parent POM in Maven).
add_library(orderbook_options INTERFACE)
# -Werror turns every warning into an error: a warning can't scroll by unseen.
# Only applies to OUR targets (gtest/benchmark don't link orderbook_options).
target_compile_options(orderbook_options INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)

if(ORDERBOOK_SANITIZE)
    target_compile_options(orderbook_options INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(orderbook_options INTERFACE
        -fsanitize=address,undefined)
endif()

# --- The order book library (all the real code lives here) -------------------
add_library(orderbook
    src/order.cpp
    src/order_book_v1.cpp
    src/order_book_v2.cpp
    src/order_book_v3.cpp)
target_include_directories(orderbook PUBLIC include)
target_link_libraries(orderbook PUBLIC orderbook_options)

# --- Executable ---------------------------------------------------------------
add_executable(orderbook_app app/main.cpp)
target_link_libraries(orderbook_app PRIVATE orderbook)

# --- Tests (GoogleTest, downloaded at configure time) ------------------------
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    SYSTEM)  # treat as third-party headers: no warnings from inside gtest
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# gtest 1.17 predates Clang 21's -Wcharacter-conversion; not our code to fix.
target_compile_options(gtest PRIVATE -Wno-character-conversion)

enable_testing()
add_executable(orderbook_tests
    tests/order_test.cpp
    tests/order_book_test.cpp)
target_link_libraries(orderbook_tests PRIVATE orderbook GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(orderbook_tests)

# --- Benchmarks (Google Benchmark). Only meaningful in the release preset! ---
FetchContent_Declare(benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
    SYSTEM)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

add_executable(orderbook_bench bench/order_book_bench.cpp)
target_link_libraries(orderbook_bench PRIVATE orderbook benchmark::benchmark)
