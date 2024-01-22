#include <tbb/parallel_for_each.h>

#include <cstddef>
#include "flavors/RLE.hpp"

#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
#include "./flavors/compintrin.cpp"
#include "./flavors/neon.cpp"
#include "./flavors/plain.cpp"
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
static constexpr uint64_t NUM_UNIQUE = 3;

template <typename DecompressFn>
class RleBench {
 protected:
  RLE<INTEGER, DecompressFn> rle_;
  std::vector<INTEGER> out_;
  RLEStructure<INTEGER>* dest_;

 public:
  RleBench() : rle_(RLE<INTEGER, DecompressFn>()) { dest_ = new RLEStructure<INTEGER>(); }

  void measure(std::vector<INTEGER>& in, size_t iterations, PerfEvent& event) {
    // results
    dest_->data = new INTEGER[in.size() * 2];
    rle_.compress(reinterpret_cast<INTEGER*>(in.data()), nullptr, dest_, in.size(), 0);

    out_.clear();
    out_.reserve(in.size() + SIMD_EXTRA_ELEMENTS(INTEGER));

    for (uint64_t i = 0; i < iterations; i++) {
      {
        PerfEventBlock b(event, in.size());
        rle_.decompress(reinterpret_cast<INTEGER*>(out_.data()), nullptr, dest_, in.size(), 0);
      }
    }

    delete[] dest_->data;
  }

  bool _verify(std::vector<INTEGER>& in, std::vector<INTEGER>& out) {
    for (size_t i = 0; i < in.size(); ++i) {
      if (in[i] != out_[i]) {
        return false;
      }
    }
    return true;
  }
};

void generate_dataset(std::vector<INTEGER>& output,
                      size_t dataset_size,
                      size_t dataset_runlength,
                      int seed = 42) {
  output.resize(dataset_size);

  std::mt19937 gen(seed);

  tbb::parallel_for(tbb::blocked_range<size_t>(0, dataset_size), [&](tbb::blocked_range<size_t> r) {
    size_t start = r.begin() - (r.begin() % dataset_runlength);

    for (size_t i = start; i < r.end() - dataset_runlength; ++i) {
      auto number = static_cast<INTEGER>(gen() % NUM_UNIQUE);

      for (auto j = 0u; j != dataset_runlength; ++j, i++) {
        output[i] = number;
      }
    }
  });
}

// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
   // ------------------------------------------------
using namespace btrblocks_simd_comparison;

int main(int argc, char** argv) {
  std::vector<INTEGER> dataset;

  PerfEvent e;

  std::vector<uint32_t> datasetSizes = { (1 << 20) / sizeof(INTEGER),  (1 << 30) / sizeof(INTEGER) };

  for (auto& datasetSize : datasetSizes) {
    e.setParam("item_size", datasetSize);

  for (size_t r = 5; r < 8; r++) {
    e.setParam("runlength", 2 << r);

    generate_dataset(dataset, datasetSize,
                     2 << r);

    e.setParam("scheme", "naive");
    RleBench<naive_rle_decompression<INTEGER>>().measure(dataset, NUM_ITERATIONS, e);
#if defined(__GNUC__)
    e.setParam("scheme", "comp_intrin_128");
    RleBench<compintrin_rle_decompression<INTEGER, 4>>().measure(dataset, NUM_ITERATIONS, e);
#if defined(__AVX2__)
    e.setParam("scheme", "comp_intrin_256");
    RleBench<compintrin_rle_decompression<INTEGER, 8>>().measure(dataset, NUM_ITERATIONS, e);
    e.setParam("scheme", "avx2");
    RleBench<avx2_rle_decompression<INTEGER>>().measure(dataset, NUM_ITERATIONS, e);
#endif
#if defined(__AVX512VL__)
    e.setParam("scheme", "comp_intrin_512");
    RleBench<compintrin_rle_decompression<INTEGER, 16>>().measure(dataset, NUM_ITERATIONS, e);
    e.setParam("scheme", "avx512");
    RleBench<avx512_rle_decompression<INTEGER>>().measure(dataset, NUM_ITERATIONS, e);
#endif
#if defined(__ARM_NEON)
    e.setParam("scheme", "neon");
    RleBench<neon_rle_decompression<INTEGER>>().measure(dataset, NUM_ITERATIONS, e);
#endif
#if defined(__ARM_FEATURE_SVE)
    e.setParam("scheme", "sve");
    RleBench<sve_rle_decompression<INTEGER>>("sve").measure(dataset, NUM_ITERATIONS, e);
#endif
#endif
  }
  }
}