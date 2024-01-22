#pragma once

#include "RLE.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
/// AVX2 IMPLEMENTATION
template <class T, size_t VECTORSIZE>
struct compintrin_rle_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const RLEStructure<T>* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
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

template <typename VectorT, typename MaskT>
inline VectorT shuffle_vector(VectorT vec, MaskT mask) {
  // The vector element size must be equal, so we convert the mask to VecT. This is a no-op if they are the same.
  // Note that this conversion can have a runtime cost, so consider using the correct type.
  // See: https://godbolt.org/z/3xdahP67o
  return __builtin_shuffle(vec, __builtin_convertvector(mask, VectorT));
}

template <size_t VECTORSIZE>
struct compintrin_rle_decompression<INTEGER, VECTORSIZE> {
  using int32xVecSize __attribute__((vector_size(VECTORSIZE * sizeof(INTEGER)), aligned(VECTORSIZE * sizeof(INTEGER)))) = INTEGER;

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

      // set is a sequential operation

      typedef int32_t int32_VECSIZE __attribute__ ((vector_size (VECTORSIZE * sizeof(INTEGER))));

      int32xVecSize vec;
      typeof (vec) v_low = {values[run_i]};
      int32_VECSIZE shufmask = {0};
      vec = shuffle_vector<int32xVecSize, int32_VECSIZE>( v_low, shufmask );

      while (write_ptr < target_ptr) {
        // store is performed in a single cycle
        store_unaligned<int32xVecSize>(write_ptr, vec);
        write_ptr += VECTORSIZE;
      }
      write_ptr = target_ptr;
    }
  }
};
#endif
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
   // -------------------------------------------------------------------------------------