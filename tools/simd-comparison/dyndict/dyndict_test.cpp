#include <memory>

#include "gtest/gtest.h"

#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
#include "./flavors/sve.cpp"
#include "./flavors/naive.cpp"

namespace btrblocks_simd_comparison {

static constexpr int32_t NUM_LENGTH = 1 << 15;
static constexpr int32_t NUM_UNIQUE = 5;

using ::testing::TestWithParam;
using ::testing::Values;

template <typename DecompressFn>
class DynDictTest : public ::testing::Test {
 protected:
  DynDictTest() : dyndict_(TDynamicDictionary<INTEGER, SInteger32Stats, DecompressFn>()) {
    inSize_ = NUM_LENGTH;
    out_ = new INTEGER[NUM_LENGTH + SIMD_EXTRA_ELEMENTS(INTEGER)];
    dictStruc_ = new DynamicDictionaryStructure();
    dictStruc_->data = new btrblocks::u8[NUM_LENGTH * 2 * sizeof(INTEGER)];
  }

  ~DynDictTest() override { delete[] out_; delete[] dictStruc_->data; delete dictStruc_; }

  TDynamicDictionary<INTEGER, SInteger32Stats, DecompressFn> dyndict_;
  INTEGER* out_;
  DynamicDictionaryStructure* dictStruc_;
  size_t inSize_;

 public:
  bool _verify(std::vector<INTEGER> &in) {
    SInteger32Stats stats = SInteger32Stats::generateStats(in.data(), nullptr, in.size());

    dyndict_.compressColumn(reinterpret_cast<INTEGER *>(in.data()), nullptr, reinterpret_cast<unsigned char*>(dictStruc_), stats, 0);
    dyndict_.decompressColumn(out_, nullptr, reinterpret_cast<unsigned char*>(dictStruc_), NUM_LENGTH, 0);

    for (size_t i = 0; i < in.size(); ++i) {
      if (in[i] != out_[i]) {
        return false;
      }
    }
    return true;
  }
};

using testing::Types;

TYPED_TEST_CASE_P(DynDictTest);

TYPED_TEST_P(DynDictTest, Basic) {
  std::vector<INTEGER> in;
  for (int32_t i = 0; i < NUM_LENGTH; ++i) {
    in.push_back(i);
  }

  EXPECT_TRUE(this->_verify(in));
}

template <typename T>
void generateRandomData(std::vector<T>& v,
                        size_t size,
                        size_t unique,
                        int seed = 42) {
  v.resize(size);
  std::mt19937 gen(seed);
  for (auto i = 0u; i < size; ++i) {
    auto number = static_cast<T>(gen() % unique);
    v[i] = number;
  }
}

TYPED_TEST_P(DynDictTest, Random) {
  std::vector<INTEGER> in;
  generateRandomData(in, NUM_LENGTH, NUM_UNIQUE);

  EXPECT_TRUE(this->_verify(in));
}

TYPED_TEST_P(DynDictTest, FastPackZeros) {
  std::vector<INTEGER> in;
  for (uint64_t i = 0; i < NUM_LENGTH; ++i) {
    in.push_back(0);
  }
  EXPECT_TRUE(this->_verify(in));
}

REGISTER_TYPED_TEST_CASE_P(DynDictTest,
                           Basic, Random, FastPackZeros);

INSTANTIATE_TYPED_TEST_CASE_P(DYNDICT_NAIVE, DynDictTest, naive_dyndict_decompression<INTEGER>);
#if defined(__GNUC__)
/*typedef compintrin_rle_decompression<INTEGER, 4> compIntrin4Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_128, RleTest, compIntrin4Flavor); */
#if defined(__AVX2__)
/*typedef compintrin_rle_decompression<INTEGER, 8> compIntrin8Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_256, RleTest, compIntrin8Flavor);*/
INSTANTIATE_TYPED_TEST_CASE_P(DYNDICT_AVX2, DynDictTest, avx2_dyndict_decompression<INTEGER>);
#endif
#if defined(__AVX512VL__)
typedef compintrin_rle_decompression<INTEGER, 16> compIntrin16Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_16, RleTest, compIntrin16Flavor);
INSTANTIATE_TYPED_TEST_CASE_P(RLE_AVX512, RleTest, avx512_rle_decompression<INTEGER>);
#endif
// #if defined(__ARM_NEON)
// INSTANTIATE_TYPED_TEST_CASE_P(RLE_NEON, RleTest, neon_rle_decompression<INTEGER>);
// #endif
#if defined(__ARM_FEATURE_SVE)
INSTANTIATE_TYPED_TEST_CASE_P(DYNDICT_SVE, DynDictTest, sve_dyndict_decompression<INTEGER>);
#endif
#endif

// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
