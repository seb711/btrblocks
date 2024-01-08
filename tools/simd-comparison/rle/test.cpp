#include <memory>

#include "gtest/gtest.h"

#include "./flavors/plain.cpp"
#if defined(__GNUC__) and defined(__AVX2__)
#include "./flavors/avx2.cpp"
#endif
#if defined(__GNUC__) and defined(__AVX512VL__)
#include "./flavors/avx512.cpp"
#endif
#if defined(__GNUC__) and defined(__ARM_NEON)
#include "./flavors/neon.cpp"
#endif
#if defined(__GNUC__) and defined(__ARM_FEATURE_SVE)
#include "./flavors/sve.cpp"
#endif

namespace btrblocks_simd_comparison {

using ::testing::TestWithParam;
using ::testing::Values;

class RleSimdTest : public ::testing::TestWithParam<std::string> {
 public:
  void SetUp() override {
    std::string name = GetParam();
    if (name == "PLAIN") {
      codec = std::make_unique<PlainRLE<INTEGER>>();
    }
    #if defined(__GNUC__) and defined(__AVX2__)
    else if (name == "AVX2") {
      codec = std::make_unique<Avx2RLE<INTEGER>>();
    }
    #endif
    #if defined(__GNUC__) and defined(__AVX512VL__)
    else if (name == "AVX512") {
      codec = std::make_unique<Avx512<INTEGER>>();
    }
    #endif
    #if defined(__GNUC__) and defined(__ARM_NEON)
    else if (name == "NEON") {
      codec = std::make_unique<NeonRLE<INTEGER>>();
    }
    #endif
    #if defined(__GNUC__) and defined(__ARM_FEATURE_SVE)
    else if (name == "SVE") {
      codec = std::make_unique<SveRLE<INTEGER>>();
    }
    #endif
     else {
      std::cout << "CODEC NOT SUPPORTED: " << name << std::endl;
    }
  }


  void TearDown() override {
    in.clear();
    out.clear();
  }

 protected:
  std::unique_ptr<RLE<INTEGER>> codec;
  std::vector<INTEGER> in;
  std::vector<INTEGER> out;
  RLEStructure<INTEGER>* dest;

  void _verify() {
    size_t inSize = in.size();

    // allocate some memory for the output; if this is passed as null,
    // the compressor will allocate the memory itself, estimating required space
    // passing too little memory here can lead to a crash/UB; memory bounds are not checked.
    dest = new RLEStructure<INTEGER>();
    dest->data = new INTEGER[inSize * 2];

    codec->compress(reinterpret_cast<INTEGER *>(in.data()), nullptr, dest, inSize, 0);

    out.reserve(inSize + SIMD_EXTRA_ELEMENTS(INTEGER));

    codec->decompress(reinterpret_cast<INTEGER *>(out.data()), nullptr, dest, inSize, 0);

    delete dest->data;

    bool passed = true;
    for (size_t i = 0; i < inSize; ++i) {
      if (in[i] != out[i]) {
        passed = false;
      }
      EXPECT_EQ(in[i], out[i]);
    }
    if (!passed) {
      std::cout << "Test failed with int32 input: ";
      for (size_t i = 0; i < inSize; ++i) {
        std::cout << in[i] << " ";
      }
      std::cout << '\n';
    }
  }

  template <typename T>
  btrblocks::Vector<T> generateRandomData(std::vector<T>& v,
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
};

TEST_P(RleSimdTest, increasingSequence) {
  in.clear();
  for (int i = -100; i < 156; ++i) {
    in.push_back(i);
  }

  _verify();
}

TEST_P(RleSimdTest, randomNumbers) {
  generateRandomData(in, 65536, 5, 5);
  _verify();
}

TEST_P(RleSimdTest, fastpack_zeors) {
  in.clear();
  for (int i = 0; i < 256; ++i) {
    in.push_back(0);
  }
  _verify();
}

TEST_P(RleSimdTest, fastpack_zeros_with_exceptions) {
  in.clear();
  in.push_back(1024);
  for (int i = 0; i < 254; ++i) {
    in.push_back(0);
  }
  in.push_back(1033);

  _verify();
}

INSTANTIATE_TEST_CASE_P(
    MyInstantiation,
    RleSimdTest,
    Values("NEON", "SVE", "AVX512", "AVX2", "PLAIN"));
}  // namespace btrblocks_simd_comparison