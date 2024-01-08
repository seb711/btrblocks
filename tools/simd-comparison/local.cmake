# ---------------------------------------------------------------------------
# BtrBlocks SIMD Comparison
# ---------------------------------------------------------------------------

set(BTR_SIMD_COMPARISON_DIR ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

add_executable(btrrlesimd
        ${BTR_SIMD_COMPARISON_DIR}/test_driver.cpp
        ${BTR_SIMD_COMPARISON_DIR}/rle/test.cpp)
target_link_libraries(btrrlesimd btrblocks gtest gmock Threads::Threads)
enable_testing()
add_test("btrsimdtesting" btrrlesimd)