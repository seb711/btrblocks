#include <tbb/parallel_for_each.h>

#include <cstddef>
#include "DynamicDictionary.hpp"

#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
#include "./flavors/naive.cpp"
#include "./flavors/sve.cpp"
#include "common/PerfEvent.hpp"
#include "storage/MMapVector.hpp"
// -------------------------------------------------------------------------------------
namespace btrblocks_simd_comparison {
// -------------------------------------------------------------------------------------
// what are we going to do?
// - we need to generate the datasets
//      - for the datasets we need different sizes
//          - runlengths of 2/4/8
// - we need a loop for benching all different architectures
// - we need different sizes of datasets

static constexpr uint64_t NUM_ITERATIONS = 3;
static constexpr uint64_t NUM_UNIQUE = 1 << 24;

template <typename DecompressFn>
class DynDictBench {
 protected:
  TDynamicDictionary<INTEGER, SInteger32Stats, DecompressFn> dyndict_;

 public:
  DynDictBench() : dyndict_(TDynamicDictionary<INTEGER, SInteger32Stats, DecompressFn>()) {}

  void measure(INTEGER* in,
               DynamicDictionaryStructure* dest,
               INTEGER* out,
               SInteger32Stats stats,
               size_t iterations,
               PerfEvent& event) {
    // results
    dyndict_.compressColumn(in, nullptr, reinterpret_cast<unsigned char*>(dest), stats, 0);

    for (uint64_t i = 0; i < iterations; i++) {
      {
        PerfEventBlock b(event, stats.tuple_count);
        dyndict_.decompressColumn(out, nullptr, reinterpret_cast<unsigned char*>(dest),
                                  stats.tuple_count, 0);
      }
    }
  }

  bool _verify(std::vector<INTEGER>& in, std::vector<INTEGER>& out) {
    for (size_t i = 0; i < in.size(); ++i) {
      if (in[i] != out[i]) {
        return false;
      }
    }
    return true;
  }
};

template <typename T>
void generate_dataset(INTEGER* output, uint64_t dataset_size, unsigned unique, int seed = 42) {
  std::mt19937 gen(seed);

  tbb::parallel_for(tbb::blocked_range<size_t>(0, dataset_size), [&](tbb::blocked_range<size_t> r) {
    for (size_t i = r.begin(); i < r.end(); ++i) {
      output[i] = static_cast<T>(gen() % unique);
    }
  });
}

// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
   // ------------------------------------------------
using namespace btrblocks_simd_comparison;

int main(int argc, char** argv) {
  PerfEvent e;

  unsigned unique = 1 << 12;

  std::vector<uint64_t> datasetSizes = {(1 << 24),
                                        (static_cast<uint64_t>(1) << static_cast<uint64_t>(30))};

  for (auto& datasetSize : datasetSizes) {
    uint64_t tuple_count = datasetSize / 4;
    // reserve here the correct space
    auto* in = new INTEGER[tuple_count];
    auto* out = new INTEGER[tuple_count + SIMD_EXTRA_ELEMENTS(INTEGER)];
    auto* dictStruc_ = new DynamicDictionaryStructure();
    dictStruc_->data = new btrblocks::u8[(unique + tuple_count) * sizeof(INTEGER)];

    e.setParam("item_size", tuple_count);

    generate_dataset<INTEGER>(in, tuple_count, unique);

    SInteger32Stats stats = SInteger32Stats::generateStats(in, nullptr, tuple_count);

#if defined(__ARM_ARCH)
    e.setParam("arch", "arm");
#elif defined(__x86_64__)
    e.setParam("arch", "x86");
#endif

    e.setParam("scheme", "naive");
    DynDictBench<naive_dyndict_decompression<INTEGER>>().measure(in, dictStruc_, out, stats,
                                                                 NUM_ITERATIONS, e);
#if defined(__GNUC__)
#if defined(__AVX2__)
    e.setParam("scheme", "avx2");
    DynDictBench<avx2_dyndict_decompression<INTEGER>>().measure(in, dictStruc_, out, stats,
                                                                NUM_ITERATIONS, e);
#endif
#if defined(__AVX512VL__)
    e.setParam("scheme", "avx512");
    RleBench<avx512_dyndict_decompression<INTEGER>>().measure(in, dictStruc_, out, stats,
                                                              NUM_ITERATIONS, e);
#endif
#if defined(__ARM_FEATURE_SVE2)
    e.setParam("scheme", "sve");
    RleBench<sve_dyndict_decompression<INTEGER>>().measure(in, dictStruc_, out, stats,
                                                           NUM_ITERATIONS, e);
#endif
#endif

    // delete allocated space
    delete[] in;
    delete[] out;
    delete[] dictStruc_->data;
    delete dictStruc_;
  }
}