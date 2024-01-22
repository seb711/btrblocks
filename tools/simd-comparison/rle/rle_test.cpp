#include <memory>

#include "gtest/gtest.h"

#include "./flavors/plain.cpp"
#include "./flavors/compintrin.cpp"
#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
#include "./flavors/neon.cpp"
#include "./flavors/sve.cpp"

namespace btrblocks_simd_comparison {

static constexpr int32_t NUM_LENGTH = 1 << 15;
static constexpr int32_t NUM_RUNLENGTH = 5;
static constexpr int32_t NUM_UNIQUE = 5;

using ::testing::TestWithParam;
using ::testing::Values;

template <typename DecompressFn>
class RleTest : public ::testing::Test {
 protected:
  RleTest() : rle_(RLE<INTEGER, DecompressFn>()) {
          inSize_ = NUM_LENGTH;
          out_ = new INTEGER[NUM_LENGTH + SIMD_EXTRA_ELEMENTS(INTEGER)];
          dest_ = new RLEStructure<INTEGER>();
          dest_->data = new INTEGER[NUM_LENGTH * 2];
  }

  ~RleTest() override { delete[] out_; delete[] dest_->data; delete dest_; }

  RLE<INTEGER, DecompressFn> rle_;
  INTEGER* out_;
  RLEStructure<INTEGER>* dest_;
  size_t inSize_;

 public:
  bool _verify(std::vector<INTEGER> &in) {
    rle_.compress(reinterpret_cast<INTEGER *>(in.data()), nullptr, dest_, NUM_LENGTH, 0);
    rle_.decompress(out_, nullptr, dest_, NUM_LENGTH, 0);

    for (size_t i = 0; i < in.size(); ++i) {
      if (in[i] != out_[i]) {
        return false;
      }
    }
    return true;
  }
};

using testing::Types;

TYPED_TEST_CASE_P(RleTest);

TYPED_TEST_P(RleTest, Basic) {
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
                        size_t runlength,
                        int seed = 42) {
  v.resize(size);
  std::mt19937 gen(seed);
  for (auto i = 0u; i < size - runlength; ++i) {
    auto number = static_cast<T>(gen() % unique);
    for (auto j = 0u; j != runlength; ++j, ++i) {
      v[i] = number;
    }
  }
}

TYPED_TEST_P(RleTest, Random) {
  std::vector<INTEGER> in;
  generateRandomData(in, NUM_LENGTH, NUM_UNIQUE, NUM_RUNLENGTH);

  EXPECT_TRUE(this->_verify(in));
}

TYPED_TEST_P(RleTest, FastPackZeros) {
  std::vector<INTEGER> in;
  for (uint64_t i = 0; i < NUM_LENGTH; ++i) {
    in.push_back(0);
  }
  EXPECT_TRUE(this->_verify(in));
}

TYPED_TEST_P(RleTest, FastPackZerosException) {
  std::vector<INTEGER> in;
  in.push_back(1024);
  for (uint64_t i = 0; i < NUM_LENGTH - 2; ++i) {
    in.push_back(0);
  }
  in.push_back(1033);

  EXPECT_TRUE(this->_verify(in));
}

REGISTER_TYPED_TEST_CASE_P(RleTest,
                            Basic, Random, FastPackZeros, FastPackZerosException);

INSTANTIATE_TYPED_TEST_CASE_P(RLE_NAIVE, RleTest, naive_rle_decompression<INTEGER>);
#if defined(__GNUC__)
typedef compintrin_rle_decompression<INTEGER, 4> compIntrin4Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_128, RleTest, compIntrin4Flavor);
#if defined(__AVX2__)
typedef compintrin_rle_decompression<INTEGER, 8> compIntrin8Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_256, RleTest, compIntrin8Flavor);
INSTANTIATE_TYPED_TEST_CASE_P(RLE_AVX2, RleTest, avx2_rle_decompression<INTEGER>);
#endif
#if defined(__AVX512VL__)
typedef compintrin_rle_decompression<INTEGER, 16> compIntrin16Flavor;
INSTANTIATE_TYPED_TEST_CASE_P(RLE_COMP_16, RleTest, compIntrin16Flavor);
INSTANTIATE_TYPED_TEST_CASE_P(RLE_AVX512, RleTest, avx512_rle_decompression<INTEGER>);
#endif
#if defined(__ARM_NEON)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_NEON, RleTest, neon_rle_decompression<INTEGER>);
#endif
#if defined(__ARM_FEATURE_SVE)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_SVE, RleTest, sve_rle_decompression<INTEGER>);
#endif
#endif

// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
