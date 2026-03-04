# Include FetchContent to download GTest and Google Benchmark
include(FetchContent)

# -----------------
# Google Test
# -----------------
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -----------------
# Google Benchmark
# -----------------
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark internal tests" FORCE)
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "Disable benchmark documentation" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# -----------------
# Test Executables
# -----------------

# Unit Tests
add_executable(test_matching test_matching.cpp)
target_link_libraries(test_matching
  PRIVATE
    exchange_core
    gtest_main
)
include(GoogleTest)
gtest_discover_tests(test_matching)

# Benchmarks
add_executable(benchmark_matching benchmark.cpp)
target_link_libraries(benchmark_matching
  PRIVATE
    exchange_core
    benchmark::benchmark
)
