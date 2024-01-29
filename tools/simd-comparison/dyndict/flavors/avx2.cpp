#pragma once

#include "../DynamicDictionary.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
/// AVX2 IMPLEMENTATION
template <class T>
struct avx2_dyndict_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const btrblocks::u8* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

#if defined(__AVX2__)
template <>
struct avx2_dyndict_decompression<INTEGER> {
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
    if (tuple_count >= 32) {
      while (i < tuple_count - 31) {
        // Load codes.
        __m256i codes_0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(codes + 0));
        __m256i codes_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(codes + 8));
        __m256i codes_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(codes + 16));
        __m256i codes_3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(codes + 24));

        // Gather values.
        __m256i values_0 = _mm256_i32gather_epi32(dict, codes_0, 4);
        __m256i values_1 = _mm256_i32gather_epi32(dict, codes_1, 4);
        __m256i values_2 = _mm256_i32gather_epi32(dict, codes_2, 4);
        __m256i values_3 = _mm256_i32gather_epi32(dict, codes_3, 4);

        // store values
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest + 0), values_0);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest + 8), values_1);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest + 16), values_2);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest + 24), values_3);

        dest += 32;
        codes += 32;
        i += 32;
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