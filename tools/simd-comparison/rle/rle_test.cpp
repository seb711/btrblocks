#pragma once

#include <memory>

#include "gtest/gtest.h"

#include "./flavors/plain.cpp"
#include "./flavors/avx2.cpp"
#include "./flavors/avx512.cpp"
#include "./flavors/neon.cpp"
#include "./flavors/sve.cpp"

namespace btrblocks_simd_comparison {

static constexpr uint64_t NUM_LENGTH = 2 << 14;
static constexpr uint64_t NUM_RUNLENGTH = 5;
static constexpr uint64_t NUM_UNIQUE = 5;

using ::testing::TestWithParam;
using ::testing::Values;

template <typename DecompressFn>
class RleTest : public ::testing::Test {
 protected:
  RleTest() : rle_(RLE<INTEGER, DecompressFn>()) {
          inSize_ = NUM_LENGTH;
          out_.reserve(NUM_LENGTH * sizeof(INTEGER) + SIMD_EXTRA_ELEMENTS(INTEGER));
          dest_ = new RLEStructure<INTEGER>();
          dest_->data = new INTEGER[NUM_LENGTH * 2];
  }

  ~RleTest() override { delete dest_->data; }

  RLE<INTEGER, DecompressFn> rle_;
  std::vector<INTEGER> out_;
  RLEStructure<INTEGER>* dest_;
  size_t inSize_;

 public:
  bool _verify(std::vector<INTEGER> &in) {
    rle_.compress(reinterpret_cast<INTEGER *>(in.data()), nullptr, dest_, in.size(), 0);
    rle_.decompress(reinterpret_cast<INTEGER *>(out_.data()), nullptr, dest_, in.size(), 0);

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
  for (uint64_t i = 0; i < NUM_LENGTH; ++i) {
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

#if defined(__GNUC__) and defined(__AVX2__)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_AVX2, RleTest, avx2_rle_decompression<INTEGER>);
#endif
#if defined(__GNUC__) and defined(__AVX512VL__)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_AVX512, RleTest, avx512_rle_decompression<INTEGER>);
#endif
#if defined(__GNUC__) and defined(__ARM_NEON)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_NEON, RleTest, neon_rle_decompression<INTEGER>);
#endif
#if defined(__GNUC__) and defined(__ARM_FEATURE_SVE)
INSTANTIATE_TYPED_TEST_CASE_P(RLE_SVE, RleTest, sve_rle_decompression<INTEGER>);
#endif

// -------------------------------------------------------------------------------------
}  // namespace btrblocks_simd_comparison
// -------------------------------------------------------------------------------------
