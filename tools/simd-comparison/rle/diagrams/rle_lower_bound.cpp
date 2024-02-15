#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <stdint.h>    // integer types
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>  // gettime
#include <algorithm>   // std::random_shuffle
#include <iostream>
#include "./PerfEvent.hpp"
#include <sys/mman.h>
#include <sys/wait.h>
#include <random>
#include <malloc.h>
#include <immintrin.h>

constexpr bool huge = true; 

static double gettime(void) {
   struct timeval now_tv;
   gettimeofday (&now_tv,NULL);
   return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}

#ifdef USE_NON_TEMPORAL
const size_t alignment = 32;
void *malloc(size_t size)
{
  size = ((size - 1) / alignment + 1) * alignment;
  return memalign(alignment, size);
}
#endif

int main(int argc,char** argv) {
   if (argc < 2) {
      printf("Usage: %s <uint64 count> <uint64 rle> <uint64 iterations> <uint64 repeat>\n", argv[0]);
      return 1;
   }
   uint64_t n=atof(argv[1]);
   uint32_t rle=atof(argv[2]);
   uint32_t iterations=atof(argv[3]);
   uint32_t repeats=atof(argv[4]);

   size_t compr_size = n / rle;

   PerfEvent e;
   e.setParam("rle", rle);
   e.setParam("compr_size", compr_size);

   for (uint64_t k = 0; k < iterations; k++) {
     e.setParam("iteration", k);

     uint32_t* target = reinterpret_cast<uint32_t*>(malloc(n * sizeof(uint32_t)));
     // prepare the data space
     uint32_t* keys = reinterpret_cast<uint32_t*>(malloc(compr_size * sizeof(uint32_t)));
     uint32_t* values = reinterpret_cast<uint32_t*>(malloc(compr_size * sizeof(uint32_t)));

     std::mt19937 gen(0xBABE);
     for (uint64_t i = 0; i != compr_size; i++) {
       keys[i] = static_cast<uint32_t>(gen());
       values[i] = static_cast<uint32_t>(gen());
     }

     for (uint64_t i = 0; i != n; i++) {
       target[i] = static_cast<uint32_t>(gen());
     }

     for (uint64_t j = 0; j < repeats; j++) {
       uint64_t result = 0;
       uint64_t pos = 0;
       e.setParam("repeat", j);
       double start = gettime();

       uint32_t* write_ptr = target;

       {
         PerfEventBlock b(e, compr_size);

         for (uint64_t i = 0; i != compr_size; i++) {
           result ^= keys[i];
           result ^= values[i];

           uint32_t* target_ptr = write_ptr + rle;

          #if defined(__AVX2__) && defined(USE_NON_TEMPORAL)
           __m256i vec = _mm256_set1_epi32(result);
           while ((uintptr_t) write_ptr % 32 && write_ptr < target_ptr) {
             *(write_ptr++) = result;
           }

           while (write_ptr < target_ptr) {
             // store is performed in a single cycle
             assert(write_ptr < target_ptr);
             assert(((uintptr_t)write_ptr % 32) == 0);
             _mm256_stream_si256(reinterpret_cast<__m256i*>(write_ptr), vec);
             write_ptr += 8;
           }
            #else
            for (uint64_t y = 0; y != rle; y++) {
              target[pos++] = result;
            }
            #endif
         }

         e.setParam("duration", (gettime()-start));
       }
     }

     delete[] keys;
     delete[] values;
     delete[] target;
   }

   return 0;
}