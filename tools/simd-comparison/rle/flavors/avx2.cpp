#pragma once

#include "RLE.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <typename NumberType>
class Avx2RLE : public RLE<NumberType> {
  // -------------------------------------------------------------------------------------
  // -------------------------------------------------------------------------------------
  void decompress(NumberType* dest,
                       BitmapWrapper* nullmap,
                       const RLEStructure<NumberType>* src,
                       btrblocks::u32 tuple_count,
                       btrblocks::u32 level) override {
    throw Generic_Exception("Unsupported templated specialization");
  }
};

template <>
inline void Avx2RLE<INTEGER>::decompress(
    INTEGER* dest,
    BitmapWrapper*,
    const RLEStructure<INTEGER>* src,
    btrblocks::u32 tuple_count,
    btrblocks::u32 level) {
  static_assert(sizeof(*dest) == 4);

  const auto* col_struct = reinterpret_cast<const RLEStructure<INTEGER>*>(src);

  /// THINK ABOUT A BETTER SOLUTION TO DO THAT (MAYBE KIND OF A BUFFER THAT'S USED BY RLE)
  /// TODO: THIS IS HIGHLY ILLEGAL BECAUSE THE MEMORY IS NOT DEALLOCATED
  auto values = col_struct->data;

  /// TODO: THIS ALSO
  auto counts = (col_struct->data + col_struct->runs_count);

  // -------------------------------------------------------------------------------------
  auto write_ptr = dest;

  auto upper_limit = dest + tuple_count;

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
    while (write_ptr < target_ptr) {
        // store is performed in a single cycle
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(write_ptr), vec);
        write_ptr += 8;
    }
    write_ptr = target_ptr;
  }
}
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------