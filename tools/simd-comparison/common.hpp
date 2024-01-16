#ifndef BTRBLOCKS_COMMON_HPP
#define BTRBLOCKS_COMMON_HPP

#if (defined(__GNUC__) && !defined(__clang__))
#define GCC_COMPILER 1
#else
#define GCC_COMPILER 0
#endif

#if (defined(__clang__))
#define CLANG_COMPILER 1
#else
#define CLANG_COMPILER 0
#endif

#endif  // BTRBLOCKS_COMMON_HPP
