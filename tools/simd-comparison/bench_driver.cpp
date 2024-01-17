// ---------------------------------------------------------------------------
// BtrBlocks
// ---------------------------------------------------------------------------
#include "benchmark/benchmark.h"
#include <iostream>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
#ifdef BTR_USE_SIMD
  std::cout << "\033[0;35mSIMD ENABLED\033[0m" << std::endl;
#else
  std::cout << "\033[0;31mSIMD DISABLED\033[0m" << std::endl;
#endif
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
// ---------------------------------------------------------------------------
