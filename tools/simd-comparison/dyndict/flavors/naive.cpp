#pragma once

#include "../DynamicDictionary.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
/// AVX2 IMPLEMENTATION
template <class T>
struct naive_dyndict_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const btrblocks::u8* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

template <>
struct naive_dyndict_decompression<INTEGER> {
  __attribute__((optimize("no-tree-vectorize")))
  void operator()(INTEGER* dest,
             BitmapWrapper* nullmap,
             const btrblocks::u8* src,
             btrblocks::u32 tuple_count,
             btrblocks::u32 level) {
    static_assert(sizeof(*dest) == 4);
    auto& col_struct = *reinterpret_cast<const DynamicDictionaryStructure*>(src);

    const INTEGER* codes =  reinterpret_cast<const INTEGER*>(col_struct.data) + col_struct.codes_offset;
    auto dict = reinterpret_cast<const INTEGER*>(col_struct.data);

    for (btrblocks::u32 i = 0; i < tuple_count; i++) {
      dest[i] = dict[codes[i]];
    }
  }
};
}