#include "Truncation.hpp"
#include "common/Units.hpp"
#include "scheme/CompressionScheme.hpp"
#include "storage/Chunk.hpp"
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
namespace btrblocks::legacy::int64s {
// -------------------------------------------------------------------------------------
double Truncation16::expectedCompressionRatio(SInt64Stats& stats, u8 allowed_cascading_level) {
  return ITruncExpectedCF<u16>(stats);
}
// -------------------------------------------------------------------------------------
u32 Truncation16::compress(const INT64* src,
                           const BITMAP* nullmap,
                           u8* dest,
                           SInt64Stats& stats,
                           u8 allowed_cascading_level) {
  return ITruncCompress<u16>(src, nullmap, dest, stats);
}
// -------------------------------------------------------------------------------------
void Truncation16::decompress(INT64* dest,
                              BitmapWrapper* nullmap,
                              const u8* src,
                              u32 tuple_count,
                              u32 level) {
  ITruncDecompress<u16>(dest, nullmap, src, tuple_count, level);
}
// -------------------------------------------------------------------------------------
INT64 Truncation16::lookup(u32) {
  UNREACHABLE();
}
void Truncation16::scan(Predicate, BITMAP*, const u8*, u32) {
  UNREACHABLE();
}
// -------------------------------------------------------------------------------------
// Truncation with 8 bits
// -------------------------------------------------------------------------------------
double Truncation8::expectedCompressionRatio(SInt64Stats& stats, u8 allowed_cascading_level) {
  return ITruncExpectedCF<u8>(stats);
}
// -------------------------------------------------------------------------------------
u32 Truncation8::compress(const INT64* src,
                          const BITMAP* nullmap,
                          u8* dest,
                          SInt64Stats& stats,
                          u8 allowed_cascading_level) {
  return ITruncCompress<u8>(src, nullmap, dest, stats);
}
// -------------------------------------------------------------------------------------
void Truncation8::decompress(INT64* dest,
                             BitmapWrapper* nullmap,
                             const u8* src,
                             u32 tuple_count,
                             u32 level) {
  ITruncDecompress<u8>(dest, nullmap, src, tuple_count, level);
}
// -------------------------------------------------------------------------------------
void Truncation8::scan(Predicate, BITMAP*, const u8*, u32) {
  UNREACHABLE();
}
INT64 Truncation8::lookup(u32) {
  UNREACHABLE();
}
}  // namespace btrblocks::legacy::int64s
// -------------------------------------------------------------------------------------