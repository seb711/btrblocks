#pragma once

#include "../DynamicDictionary.hpp"
#include "extern/RoaringBitmap.hpp"

// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
/// AVX2 IMPLEMENTATION
template <class T>
struct sve_dyndict_decompression {
  uint64_t operator()(T* dest,
                      BitmapWrapper* nullmap,
                      const btrblocks::u8* src,
                      btrblocks::u32 tuple_count,
                      btrblocks::u32 level) {
    throw Generic_Exception("Not implemented");
  }
};

#if defined(__ARM_FEATURE_SVE)
template <>
struct sve_dyndict_decompression<INTEGER> {
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
    int w = svcntw();
    svbool_t tp = svptrue_b32();


    if (tuple_count >= w * 4) {
      while (i < tuple_count - (w * 4 - 1)) {
        // Load codes.
        svuint32_t codes_0 = svld1_u32(reinterpret_cast<const svuint32_t*>(tp, codes + w));
        svuint32_t codes_1 = svld1_u32(reinterpret_cast<const svuint32_t*>(tp, codes + 2 * w));
        svuint32_t codes_2 = svld1_u32(reinterpret_cast<const svuint32_t*>(tp, codes + 3 * w));
        svuint32_t codes_3 = svld1_u32(reinterpret_cast<const svuint32_t*>(tp, codes + 4 * w));

        // Gather values.
        svint32_t values_0 = svldnt1_gather_u32offset_s32(tp, dict, codes_0);
        svint32_t values_1 = svldnt1_gather_u32offset_s32(tp, dict, codes_1);
        svint32_t values_2 = svldnt1_gather_u32offset_s32(tp, dict, codes_2);
        svint32_t values_3 = svldnt1_gather_u32offset_s32(tp, dict, codes_3);

        // store values
        svst1_s32(tp, reinterpret_cast<uint32_t*>(dest + w), values_0);
        svst1_s32(tp, reinterpret_cast<uint32_t*>(dest + 2 * w), values_1);
        svst1_s32(tp, reinterpret_cast<uint32_t*>(dest + 3 * w), values_2);
        svst1_s32(tp, reinterpret_cast<uint32_t*>(dest + 4 * w), values_3);

        dest += w * 4;
        codes += w * 4;
        i += w * 4;
      }
    }

    // TODO: this could be done also with sve
    while (i < tuple_count) {
      *dest++ = dict[*codes++];
      i++;
    }
  }
};
#endif
}