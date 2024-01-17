#pragma once

#include "RLE.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
/// AVX2 IMPLEMENTATION
template <class T>
struct compintrin_rle_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<T>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {}
};

#if defined(__GNUC__)
template <typename ElementT_, size_t VECTOR_SIZE_IN_BYTES, size_t ALIGNMENT = VECTOR_SIZE_IN_BYTES>
struct GccVec {
  using ElementT = ElementT_;
  using T __attribute__((vector_size(VECTOR_SIZE_IN_BYTES), aligned(ALIGNMENT))) = ElementT;
  using UnalignedT __attribute__((vector_size(VECTOR_SIZE_IN_BYTES), aligned(1))) = ElementT;
};

template <typename VectorT>
inline void store(void* ptr, VectorT value) {
  *reinterpret_cast<VectorT*>(ptr) = value;
}

template <typename VectorT>
inline void store_unaligned(void* ptr, VectorT value) {
  using UnalignedVector __attribute__((aligned(1))) = VectorT;
  *reinterpret_cast<UnalignedVector*>(ptr) = value;
}

template <>
struct compintrin_rle_decompression<INTEGER> {
  using int32x4 = GccVec<int32_t, 16>::T;

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
      int32x4 vec = {
          values[run_i],
          values[run_i],
          values[run_i],
          values[run_i]
      };
      while (write_ptr < target_ptr) {
        // store is performed in a single cycle
        store_unaligned<int32x4>(write_ptr, vec);
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