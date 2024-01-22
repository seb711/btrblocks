#pragma once

#include "RLE.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <class T>
struct naive_rle_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<T>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

template <>
struct naive_rle_decompression<INTEGER> {
  __attribute__((optimize("no-tree-vectorize")))
  void operator()(INTEGER* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<INTEGER>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    static_assert(sizeof(*dest) == 4);

    auto values = src->data;
    auto counts = (src->data + src->runs_count);
    // -------------------------------------------------------------------------------------

    /// THIS IS THE INTERESTING PART HERE
    for (btrblocks::u32 run_i = 0; run_i < (src->runs_count); run_i++) {
      INTEGER val = values[run_i];
      auto target_ptr = dest + counts[run_i];
      while (dest != target_ptr) {
        *(dest++) = val;
      }
    }
  }
};
// -------------------------------------------------------------------------------------
}  // namespace btrblocks::integers
// -------------------------------------------------------------------------------------