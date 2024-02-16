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

#if defined(__AVX2__)
#include <immintrin.h>
#endif

constexpr bool huge = true; 

static double gettime(void) {
   struct timeval now_tv;
   gettimeofday (&now_tv,NULL);
   return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}

#if defined(NEW_MALLOC_ALLOCATION) && defined(USE_MMAP)
#error "mmap and new malloc every call doesnt work."
#endif

#ifdef USE_NON_TEMPORAL
const alignment = 32;
void *malloc(size_t size)
{
  size = ((size - 1) / 32 + 1) * 32;
  return memalign(32, size);
}
#endif

#ifdef USE_MMAP
static void* malloc(size_t size) {
   void* p =
         ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if constexpr (huge) { ::madvise(p, size, MADV_HUGEPAGE); }
   if (p == MAP_FAILED) { throw; }
   return p;
}
#endif


int main(int argc,char** argv) {
   if (argc < 2) {
      printf("Usage: %s <uint64 count> <uint64 rle> <uint64 iterations>\n", argv[0]);
      return 1;
   }
   uint64_t n=atof(argv[1]);
   uint32_t rle=atof(argv[2]);
   uint32_t iterations=atof(argv[3]);

   uint32_t* target = reinterpret_cast<uint32_t*>(malloc(n * sizeof(uint32_t)));

   #ifdef USE_MMAP
   std::cout << "using mmap..." << std::endl; 
   #endif

   PerfEvent we;
   PerfEvent re;

   for (int i = 0; i < iterations; i++) {
      #ifdef NEW_MALLOC_ALLOCATION
      delete[] target; 
      target = reinterpret_cast<uint32_t*>(malloc(n * sizeof(uint32_t)));
      #endif

      {
         PerfEventBlock b(we, n);

         double start = gettime();

         for (uint64_t i=0; i!=n; i++) {
            target[i] = i;
         }

         we.setParam("duration", (gettime()-start));
         we.setParam("throughput (GB/s)", ((sizeof(uint32_t)*n)/(1024*1024*1024))/(gettime()-start));
      }
   }

   size_t compr_size = n / rle;
   uint32_t* keys = reinterpret_cast<uint32_t*>(malloc(compr_size * sizeof(uint32_t)));
   uint32_t* values = reinterpret_cast<uint32_t*>(malloc(compr_size * sizeof(uint32_t)));

#if !defined(WRITE_ONLY)

   {
      for (uint64_t i=0; i!=compr_size; i++) {
         keys[i] = 1;
         values[i] = 2;
      }
   }

   for (uint64_t i=0; i<iterations; i++) {
      PerfEventBlock b(re, compr_size);

      uint64_t key_sum = 0;
      uint64_t values_sum = 0; 

      double start = gettime();

      for (uint64_t i=0; i!=compr_size; i++) {
         key_sum += keys[i];
         values_sum += values[i];
      }

      re.setParam("rle", rle);
      re.setParam("compr_size", compr_size);
      re.setParam("checkval", key_sum + values_sum);
      re.setParam("duration", (gettime()-start));

      re.setParam("throughput (GB/s)", ((sizeof(uint32_t)*2*compr_size)/(1024.00*1024*1024))/(gettime()-start));
   }
#endif

   #ifdef USE_MMAP
   if (munmap(keys, compr_size * sizeof(uint32_t)))
   {
      exit(EXIT_FAILURE);
   }   if (munmap(values, compr_size * sizeof(uint32_t)))
   {
      exit(EXIT_FAILURE);
   }
   if (munmap(target, n * sizeof(uint32_t)) < 0)
   {
      exit(EXIT_FAILURE);
   }
   #else
   delete[] keys; 
   delete[] values; 
   delete[] target; 
   #endif

   return 0;
}