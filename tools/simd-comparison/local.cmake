# ---------------------------------------------------------------------------
# BtrBlocks SIMD Comparison
# ---------------------------------------------------------------------------

set(BTR_SIMD_COMPARISON_DIR ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

add_executable(btr_rle_simd_tests
        ${BTR_SIMD_COMPARISON_DIR}/test_driver.cpp
        ${BTR_SIMD_COMPARISON_DIR}/rle/rle_test.cpp
        ${BTR_SIMD_COMPARISON_DIR}/dyndict/dyndict_test.cpp)
target_link_libraries(btr_rle_simd_tests btrblocks gtest gmock Threads::Threads)
enable_testing()
add_test("btr_rle_simd_tests" btr_rle_simd_tests)

add_executable(btr_rle_simd_benchmarks
        ${BTR_SIMD_COMPARISON_DIR}/rle/rle_benchmark.cpp)
target_link_libraries(btr_rle_simd_benchmarks btrblocks tbb gmock Threads::Threads)
