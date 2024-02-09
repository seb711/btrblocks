#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <stdint.h>    // integer types
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>  // gettime
#include <algorithm>   // std::random_shuffle
#include <iostream>
#include "./PerfEvent.hpp"

static double gettime(void) {
   struct timeval now_tv;
   gettimeofday (&now_tv,NULL);
   return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}


int main(int argc,char** argv) {
   if (argc < 2) {
      printf("Usage: %s <uint64 count> <uint64 rle>\n", argv[0]);
      return 1;
   }
   uint64_t n=atof(argv[1]);
   uint32_t rle=atof(argv[2]);
   uint32_t* target = new uint32_t[n];

   PerfEvent we;
   PerfEvent re;

   uint32_t counter = 0; 
   for (int i = 0; i < 10; i++) {

      PerfEventBlock b(we, n);

      double start = gettime();

      for (uint64_t i=0; i!=n; i++) {
         target[i] = i;
      }

      we.setParam("throughput (GB/s)", ((sizeof(uint32_t)*n)/(1024*1024*1024))/(gettime()-start));
   }


   size_t compr_size = n / rle;
   uint32_t* keys = new uint32_t[compr_size];
   uint32_t* values = new uint32_t[compr_size];

   {
      double start = gettime();
      for (uint64_t i=0; i!=compr_size; i++) {
         keys[i] = 1;
         values[i] = 2;
      }
   }

   for (uint64_t i=0; i<20; i++) {
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

   delete[] keys; 
   delete[] values; 

   
   delete[] target; 

   return 0;
}