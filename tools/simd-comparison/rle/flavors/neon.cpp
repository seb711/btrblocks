#pragma once

#include "RLE.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <typename NumberType>
class NeonRLE : public RLE<NumberType> {
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
inline void NeonRLE<INTEGER>::decompress(
    INTEGER* dest,
    BitmapWrapper*,
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
      vst1q_s32(reinterpret_cast<int32x4_t*>(write_ptr), vec);
      write_ptr += 8;
    }
    write_ptr = target_ptr;
  }
}
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
