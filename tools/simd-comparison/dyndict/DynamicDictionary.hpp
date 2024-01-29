#pragma once
#include "common/Utils.hpp"
#include "stats/NumberStats.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
using SInteger32Stats = btrblocks::NumberStats<btrblocks::s32>;
using INTEGER = btrblocks::s32;  // we use FOR always at the beginning so negative integers

struct __attribute__((packed)) DynamicDictionaryStructure {
  btrblocks::u8* data;
  btrblocks::u8 codes_scheme_code;
  btrblocks::u32 codes_offset;
};
// -------------------------------------------------------------------------------------
// Used for integer and doubles
template <typename NumberType, typename StatsType, typename DecompressFn>
class TDynamicDictionary {
 public:
  static inline btrblocks::u32 compressColumn(const NumberType* src,
                                              const btrblocks::BITMAP* nullmap,
                                              btrblocks::u8* dest,
                                              StatsType& stats,
                                              btrblocks::u8 allowed_cascading_level) {
    // Layout: DICT | CODES
    auto& col_struct = *reinterpret_cast<DynamicDictionaryStructure*>(dest);
    // -------------------------------------------------------------------------------------
    // Write dictionary
    auto dict_slots = reinterpret_cast<NumberType*>(col_struct.data);
    btrblocks::u32 distinct_i = 0;
    for (const auto& distinct_element : stats.distinct_values) {
      dict_slots[distinct_i] = distinct_element.first;
      distinct_i++;
    }
    // -------------------------------------------------------------------------------------
    auto dict_begin = reinterpret_cast<NumberType*>(col_struct.data);
    auto dict_end = reinterpret_cast<NumberType*>(col_struct.data) + distinct_i;
    // -------------------------------------------------------------------------------------
    std::vector<INTEGER> codes;
    for (btrblocks::u32 row_i = 0; row_i < stats.tuple_count; row_i++) {
      auto it = std::lower_bound(dict_begin, dict_end, src[row_i]);
      if (it == dict_end) {
        die_if(stats.distinct_values.find(src[row_i]) != stats.distinct_values.end());
      }
      die_if(it != dict_end);
      codes.push_back(std::distance(dict_begin, it));
    }
    // -------------------------------------------------------------------------------------
    // Compress codes
    auto write_ptr = reinterpret_cast<btrblocks::u8*>(dict_end);
    col_struct.codes_offset = distinct_i;
    btrblocks::u32 used_space;

    // memmove stuff from codes to col_struct.data
    memmove(write_ptr, codes.data(), codes.size() * sizeof(INTEGER));

    write_ptr += used_space;
    // -------------------------------------------------------------------------------------
    return write_ptr - dest;
  }
  // -------------------------------------------------------------------------------------
  DecompressFn decompressColumn{};

};  // namespace btrblocks
}