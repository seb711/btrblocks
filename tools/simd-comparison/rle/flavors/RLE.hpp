#pragma once
// -------------------------------------------------------------------------------------
#include "scheme/CompressionScheme.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
using SInteger32Stats = btrblocks::NumberStats<btrblocks::s32>;
using INTEGER = btrblocks::s32;  // we use FOR always at the beginning so negative integers
// -------------------------------------------------------------------------------------
template <class NumberType>
struct RLEStructure {
  btrblocks::u32 runs_count;
  btrblocks::u32 runs_count_offset;
  btrblocks::u8 values_scheme_code;
  btrblocks::u8 counts_scheme_code;
  NumberType* data;
};

template <class NumberType>
class RLE {
  // -------------------------------------------------------------------------------------
 public:
  virtual btrblocks::u32 compress(const NumberType* src,
               const btrblocks::BITMAP* nullmap,
               RLEStructure<NumberType>* dest,
               btrblocks::u32 tuple_count,
               btrblocks::u8 allowed_cascading_level) {
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
    auto* write_ptr = reinterpret_cast<btrblocks::u8*>(col_struct->data);

    memcpy(write_ptr, rle_values.data(), rle_values.size() * sizeof(NumberType));

    write_ptr += rle_values.size() * sizeof(NumberType);

    memcpy(write_ptr, rle_count.data(), rle_count.size() * sizeof(INTEGER));

    write_ptr += rle_count.size() * sizeof(INTEGER);

    return write_ptr - reinterpret_cast<btrblocks::u8*>(dest);;
  };
  virtual void decompress(NumberType* dest,
                  BitmapWrapper* nullmap,
                  const RLEStructure<NumberType>* src,
                  btrblocks::u32 tuple_count,
                  btrblocks::u32 level) {
    throw Generic_Exception("Unsupported templated specialization");
  };
};
// -------------------------------------------------------------------------------------
}  // namespace btrblocks::integers
// -------------------------------------------------------------------------------------
