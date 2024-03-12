#pragma once

#include "../RLE.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
/// AVX2 IMPLEMENTATION
template <class T>
struct avx2_rle_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<T>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

#if defined(__AVX2__)
template <>
struct avx2_rle_decompression<INTEGER> {
  void operator()(INTEGER* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<INTEGER>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    static_assert(sizeof(*dest) == 4);

    const auto* col_struct = reinterpret_cast<const RLEStructure<INTEGER>*>(src);

    auto values = col_struct->data;
    auto counts = (col_struct->data + col_struct->runs_count);

    // -------------------------------------------------------------------------------------
    auto write_ptr = dest;

    /// THIS IS THE INTERESTING PART HERE
    for (btrblocks::u32 run_i = 0; run_i < col_struct->runs_count; run_i++) {
      auto target_ptr = write_ptr + counts[run_i];

      /*
     * I tried several variation for vectorizing this. Using AVX2 directly is
     * the fastest even when there are many very short runs. The penalty of
     * branching simply outweighs the few instructions saved by not using AVX2
     * for short runs
       */
      // set is a sequential operation
      __m256i vec = _mm256_set1_epi32(values[run_i]);

      // 1. measure same with aligned intrinsic in store
      // 2. unrolling
      // 3. naive see what compiler does
      // make the alignment correct
      while ((uintptr_t) write_ptr % 32 && write_ptr < target_ptr) {
        *(write_ptr++) = values[run_i];
      }

      while (write_ptr < target_ptr) {
        // store is performed in a single cycle
        // non temporal memory is used
        _mm256_stream_si256(reinterpret_cast<__m256i*>(write_ptr), vec);
        write_ptr += 8;
      }
      write_ptr = target_ptr;
    }
  }
};
#endif
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------