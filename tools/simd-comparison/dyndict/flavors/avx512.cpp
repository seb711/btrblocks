#pragma once

#include "../DynamicDictionary.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
/// AVX2 IMPLEMENTATION
template <class T>
struct avx512_dyndict_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const btrblocks::u8* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

#if defined(__AVX512VL__)
template <>
struct avx512_dyndict_decompression<INTEGER> {
  void operator()(INTEGER* dest,
                  BitmapWrapper* nullmap,
                  const btrblocks::u8* src,
                  btrblocks::u32 tuple_count,
                  btrblocks::u32 level) {
    static_assert(sizeof(*dest) == 4);

    auto& col_struct = *reinterpret_cast<const DynamicDictionaryStructure*>(src);

    const INTEGER* codes =  reinterpret_cast<const INTEGER*>(col_struct.data) + col_struct.codes_offset;
    auto dict = reinterpret_cast<const INTEGER*>(col_struct.data);

    btrblocks::u32 i = 0;
    if (tuple_count >= 64) {
      while (i < tuple_count - 63) {
        // Load codes.
        __m512i codes_0 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(codes + 0));
        __m512i codes_1 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(codes + 16));
        __m512i codes_2 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(codes + 32));
        __m512i codes_3 = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(codes + 48));

        // Gather values.
        __m512i values_0 = _mm512_i32gather_epi32(codes_0, dict, 4);
        __m512i values_1 = _mm512_i32gather_epi32(codes_1, dict, 4);
        __m512i values_2 = _mm512_i32gather_epi32(codes_2, dict, 4);
        __m512i values_3 = _mm512_i32gather_epi32(codes_3, dict, 4);

        // store values
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + 0), values_0);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + 16), values_1);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + 32), values_2);
        _mm512_storeu_si512(reinterpret_cast<__m512i*>(dest + 48), values_3);

        dest += 64;
        codes += 64;
        i += 64;
      }
    }

    while (i < tuple_count) {
      *dest++ = dict[*codes++];
      i++;
    }
  }
};
#endif
}