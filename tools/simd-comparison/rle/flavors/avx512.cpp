#pragma once

#include "RLE.hpp"

#if defined(__AVX512__)
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
template <typename NumberType>
class Avx512RLE : public RLE<NumberType> {
  // -------------------------------------------------------------------------------------
  btrblocks::u32 compress(const NumberType* src,
                          const btrblocks::BITMAP* nullmap,
                          RLEStructure<NumberType>* dest,
                          btrblocks::u32 tuple_count,
                          btrblocks::u8 allowed_cascading_level) override {
    RLEStructure<NumberType>* col_struct = reinterpret_cast<RLEStructure<NumberType>*>(dest);
    // -------------------------------------------------------------------------------------
    std::vector<NumberType> rle_values;
    std::vector<btrblocks::INTEGER> rle_count;
    // -------------------------------------------------------------------------------------
    // RLE encoding
    NumberType last_item = src[0];
    btrblocks::INTEGER count = 1;
    for (uint32_t row_i = 1; row_i < tuple_count; row_i++) {
      if (src[row_i] == last_item ||
          (nullmap != nullptr && !nullmap[row_i])) {  // the null trick brought
                                                      // no compression benefits
        count++;
      } else {
        rle_count.push_back(count);
        rle_values.push_back(last_item);
        last_item = src[row_i];
        count = 1;
      }
    }
    rle_count.push_back(count);
    rle_values.push_back(last_item);
    // -------------------------------------------------------------------------------------
    col_struct->runs_count = rle_count.size();
    die_if(rle_count.size() == rle_values.size());
    // -------------------------------------------------------------------------------------
    btrblocks::u8* write_ptr = reinterpret_cast<btrblocks::u8*>(col_struct->data);

    memcpy(write_ptr, rle_values.data(), rle_values.size() * sizeof(NumberType));

    write_ptr += rle_values.size() * sizeof(NumberType);

    memcpy(write_ptr, rle_count.data(), rle_count.size() * sizeof(INTEGER));

    write_ptr += rle_count.size() * sizeof(INTEGER);

    return write_ptr - reinterpret_cast<btrblocks::u8*>(dest);
  }
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
inline void Avx512RLE<INTEGER>::decompress(
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
    __m512i vec = _mm512_set1_epi32(values[run_i]);
    while (write_ptr < target_ptr) {
      if (write_ptr + 16 <= upper_limit) {
        // store is performed in a single cycle
        _mm512_storeu_epi32(reinterpret_cast<__m512i*>(write_ptr), vec);
        write_ptr += 16;
      } else {
        while (write_ptr != target_ptr) {
          *write_ptr++ = values[run_i];
        }
      }
    }
    write_ptr = target_ptr;
  }
}
// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
   // -------------------------------------------------------------------------------------
#endif