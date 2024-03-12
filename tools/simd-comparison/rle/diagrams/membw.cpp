///usr/bin/env -S g++ "$0" -ltbb -O3 -o /tmp/membw.out && exec /tmp/membw.out "$@"
#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <stdint.h>    // integer types
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>  // gettime
#include <algorithm>   // std::random_shuffle
#include <tbb/tbb.h>
#include <pthread.h>
#include <atomic>
#include <iostream>

using namespace tbb;

static double gettime(void) {
   struct timeval now_tv;
   gettimeofday (&now_tv,NULL);
   return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}


int main(int argc,char** argv) {
   if (argc < 2) {
      printf("Usage: %s <uint64 count> <threads>\n", argv[0]);
      return 1;
   }
   uint64_t n=atof(argv[1]);
   uint64_t* keys = new uint64_t[n];
   //tbb::task_scheduler_init init(atoi(argv[2]));
   oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism, atoi(argv[2]));
   static affinity_partitioner ap;

   while (true) {
	   for (int rep=0;rep!=3;++rep) {
         double start = gettime();
         tbb::parallel_for(tbb::blocked_range<uint64_t> (0, n), [&] (const tbb::blocked_range<uint64_t>& range) {
                                                                  for (uint64_t i=range.begin(); i!=range.end(); i++) {
                                                                     keys[i] =i;
                                                                  }}, ap);
         printf("%f\n",((sizeof(uint64_t)*n)/(1024*1024*1024))/(gettime()-start));
      }

      for (int rep=0;rep!=3;++rep) {
         std::atomic<uint64_t> s(0);
         double start = gettime();
         tbb::parallel_for(tbb::blocked_range<uint64_t> (0, n), [&] (const tbb::blocked_range<uint64_t>& range) {
                                                                  uint64_t sum = 0;
                                                                  for (uint64_t i=range.begin(); i!=range.end(); i++) {
                                                                     sum += keys[i];
                                                                  }
                                                                  s += sum;
                                                               }, ap);
         printf("%f %ld\n",(static_cast<double>(sizeof(uint64_t)*n)/(1024ull*1024*1024))/(gettime()-start), s.load());
      }
   }

   return 0;
}
