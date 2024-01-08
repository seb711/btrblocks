#pragma once

#include "RLE.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <typename NumberType>
class SveRLE : public RLE<NumberType> {
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
inline void SveRLE<INTEGER>::decompress(
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

  int w = svcntw();
  svbool_t tp = svptrue_b32();

  /// THIS IS THE INTERESTING PART HERE
  for (btrblocks::u32 run_i = 0; run_i < col_struct->runs_count; run_i++) {
    auto target_ptr = write_ptr + counts[run_i];

    // set is a sequential operation
    svint32_t vec = svdup_n_s32_x(tp, values[run_i]);
    while (write_ptr < target_ptr) {
      // store is performed in a single cycle
      // TODO: check if tp can be masked
      svst1_s32(tp, reinterpret_cast<INTEGER*>(write_ptr), vec);
      write_ptr += w;
    }
    write_ptr = target_ptr;
  }
}
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
