#pragma once

#include "RLE.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
/// AVX2 IMPLEMENTATION
template <class T>
struct neon_rle_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<T>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {}
};

#if defined(__ARM_NEON)
template <>
struct neon_rle_decompression<INTEGER> {
  uint64_t operator()(INTEGER* dest,
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

    auto upper_limit = dest + tuple_count;

    /// THIS IS THE INTERESTING PART HERE
    for (btrblocks::u32 run_i = 0; run_i < col_struct->runs_count; run_i++) {
      auto target_ptr = write_ptr + counts[run_i];

      // set is a sequential operation
      int32x4_t vec = vdupq_n_s32(values[run_i]);
      while (write_ptr < target_ptr) {
        // store is performed in a single cycle
        vst1q_s32(reinterpret_cast<INTEGER*>(write_ptr), vec);
        write_ptr += 4;
      }
      write_ptr = target_ptr;
    }
  }
};
#endif
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
