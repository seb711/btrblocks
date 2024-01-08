#pragma once

#include "RLE.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <typename NumberType>
class PlainRLE : public RLE<NumberType> {
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
inline void PlainRLE<INTEGER>::decompress(
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

  /// THIS IS THE INTERESTING PART HERE
  for (btrblocks::u32 run_i = 0; run_i < col_struct->runs_count; run_i++) {
    auto val = values[run_i];
    auto target_ptr = write_ptr + counts[run_i];
    while (write_ptr != target_ptr) {
      *write_ptr++ = val;
    }
  }
}
// -------------------------------------------------------------------------------------
}  // namespace btrblocks::integers
// -------------------------------------------------------------------------------------