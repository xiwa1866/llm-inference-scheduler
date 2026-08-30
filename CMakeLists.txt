cmake_minimum_required(VERSION 3.20)

project(LLM_scheduler
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Library
# -----------------------------------------------------------------------------

add_library(scheduler
    src/scheduler.cpp
    src/worker_pool.cpp
    src/kv_cache.cpp
    src/metrics.cpp
)

target_include_directories(scheduler
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------------------------------------------------------
# Example
# -----------------------------------------------------------------------------

add_executable(basic_scheduler
    examples/basic_scheduler.cpp
)

target_link_libraries(basic_scheduler
    PRIVATE scheduler
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

# enable_testing()

# add_executable(scheduler_test
#     tests/scheduler_test.cpp
# )

# target_link_libraries(scheduler_test
#     PRIVATE scheduler
# )

# add_test(
#     NAME scheduler_test
#     COMMAND scheduler_test
# )

# add_executable(kv_cache_test
#     tests/kv_cache_test.cpp
# )

# target_link_libraries(kv_cache_test
#     PRIVATE scheduler
# )

# add_test(
#     NAME kv_cache_test
#     COMMAND kv_cache_test
# )

# add_executable(concurrency_test
#     tests/concurrency_test.cpp
# )

# target_link_libraries(concurrency_test
#     PRIVATE scheduler
# )

# add_test(
#     NAME concurrency_test
#     COMMAND concurrency_test
# )

# # -----------------------------------------------------------------------------
# # Benchmark
# # -----------------------------------------------------------------------------

# add_executable(benchmark
#     bench/benchmark.cpp
# )

# target_link_libraries(benchmark
#     PRIVATE scheduler
# )