#include <tbb/parallel_for_each.h>

#include <cstddef>
#include "flavors/RLE.hpp"

#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
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

static constexpr uint64_t NUM_UNIQUE = 5;

template <typename DecompressFn>
class RleBench {
 protected:
  RLE<INTEGER, DecompressFn> rle_;
  std::vector<INTEGER> out_;
  RLEStructure<INTEGER>* dest_;
  std::string impl_name;

 public:
  RleBench(std::string impl_name) : rle_(RLE<INTEGER, DecompressFn>()), impl_name(impl_name) { dest_ = new RLEStructure<INTEGER>(); }

  void measure(std::vector<INTEGER>& in, size_t iterations) {
    // results
    PerfEvent e;

    e.setParam("scheme", impl_name);

    dest_->data = new INTEGER[in.size() * 2];
    rle_.compress(reinterpret_cast<INTEGER*>(in.data()), nullptr, dest_, in.size(), 0);

    out_.clear();
    out_.reserve(in.size() + SIMD_EXTRA_ELEMENTS(INTEGER));

    for (int i = 0; i < iterations; i++) {
      {
        PerfEventBlock b(e, in.size());
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

int main(int argc, char **argv) {
  std::vector<INTEGER> dataset;

  size_t entries = (static_cast<size_t>(512 * (1 << 20))) / sizeof(INTEGER);

  generate_dataset(dataset, entries, 64);

  RleBench<naive_rle_decompression<INTEGER>>("naive").measure(dataset, 5);

#if defined(__GNUC__) and defined(__AVX2__)
  RleBench<avx2_rle_decompression<INTEGER>>("avx2").measure(dataset, 5);
#endif
#if defined(__GNUC__) and defined(__AVX512VL__)
  RleBench<avx512_rle_decompression<INTEGER>>("avx512").measure(dataset, 5);
#endif
#if defined(__GNUC__) and defined(__ARM_NEON)
  RleBench<neon_rle_decompression<INTEGER>>("neon").measure(dataset, 5);
#endif
#if defined(__GNUC__) and defined(__ARM_FEATURE_SVE)
  RleBench<sve_rle_decompression<INTEGER>>("sve").measure(dataset, 5);
#endif
}