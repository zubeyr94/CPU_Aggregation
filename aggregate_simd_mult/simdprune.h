#include "simdprune_tables.h"

static inline __m256i prune256_epi32(__m256i x, int mask) {
  return _mm256_permutevar8x32_epi32(
      x, _mm256_loadu_si256((const __m256i *)mask256_epi32 + mask));
}
