#include <stdint.h>
#include <immintrin.h>
static const uint32_t mask256_epi32[] = {
0,1,2,3,4,5,6,7,
1,2,3,4,5,6,7,0,
0,2,3,4,5,6,7,1,
2,3,4,5,6,7,1,1,
0,1,3,4,5,6,7,2,
1,3,4,5,6,7,2,2,
0,3,4,5,6,7,2,2,
3,4,5,6,7,2,2,2,
0,1,2,4,5,6,7,3,
1,2,4,5,6,7,3,3,
0,2,4,5,6,7,3,3,
2,4,5,6,7,3,3,3,
0,1,4,5,6,7,3,3,
1,4,5,6,7,3,3,3,
0,4,5,6,7,3,3,3,
4,5,6,7,3,3,3,3,
0,1,2,3,5,6,7,4,
1,2,3,5,6,7,4,4,
0,2,3,5,6,7,4,4,
2,3,5,6,7,4,4,4,
0,1,3,5,6,7,4,4,
1,3,5,6,7,4,4,4,
0,3,5,6,7,4,4,4,
3,5,6,7,4,4,4,4,
0,1,2,5,6,7,4,4,
1,2,5,6,7,4,4,4,
0,2,5,6,7,4,4,4,
2,5,6,7,4,4,4,4,
0,1,5,6,7,4,4,4,
1,5,6,7,4,4,4,4,
0,5,6,7,4,4,4,4,
5,6,7,4,4,4,4,4,
0,1,2,3,4,6,7,5,
1,2,3,4,6,7,5,5,
0,2,3,4,6,7,5,5,
2,3,4,6,7,5,5,5,
0,1,3,4,6,7,5,5,
1,3,4,6,7,5,5,5,
0,3,4,6,7,5,5,5,
3,4,6,7,5,5,5,5,
0,1,2,4,6,7,5,5,
1,2,4,6,7,5,5,5,
0,2,4,6,7,5,5,5,
2,4,6,7,5,5,5,5,
0,1,4,6,7,5,5,5,
1,4,6,7,5,5,5,5,
0,4,6,7,5,5,5,5,
4,6,7,5,5,5,5,5,
0,1,2,3,6,7,4,4,
1,2,3,6,7,4,4,4,
0,2,3,6,7,4,4,4,
2,3,6,7,4,4,4,4,
0,1,3,6,7,4,4,4,
1,3,6,7,4,4,4,4,
0,3,6,7,4,4,4,4,
3,6,7,4,4,4,4,4,
0,1,2,6,7,3,3,3,
1,2,6,7,3,3,3,3,
0,2,6,7,3,3,3,3,
2,6,7,3,3,3,3,3,
0,1,6,7,2,2,2,2,
1,6,7,2,2,2,2,2,
0,6,7,1,1,1,1,1,
6,7,1,1,1,1,1,1,
0,1,2,3,4,5,7,6,
1,2,3,4,5,7,6,6,
0,2,3,4,5,7,6,6,
2,3,4,5,7,6,6,6,
0,1,3,4,5,7,6,6,
1,3,4,5,7,6,6,6,
0,3,4,5,7,6,6,6,
3,4,5,7,6,6,6,6,
0,1,2,4,5,7,6,6,
1,2,4,5,7,6,6,6,
0,2,4,5,7,6,6,6,
2,4,5,7,6,6,6,6,
0,1,4,5,7,6,6,6,
1,4,5,7,6,6,6,6,
0,4,5,7,6,6,6,6,
4,5,7,6,6,6,6,6,
0,1,2,3,5,7,6,6,
1,2,3,5,7,6,6,6,
0,2,3,5,7,6,6,6,
2,3,5,7,6,6,6,6,
0,1,3,5,7,6,6,6,
1,3,5,7,6,6,6,6,
0,3,5,7,6,6,6,6,
3,5,7,6,6,6,6,6,
0,1,2,5,7,6,6,6,
1,2,5,7,6,6,6,6,
0,2,5,7,6,6,6,6,
2,5,7,6,6,6,6,6,
0,1,5,7,6,6,6,6,
1,5,7,6,6,6,6,6,
0,5,7,6,6,6,6,6,
5,7,6,6,6,6,6,6,
0,1,2,3,4,7,5,5,
1,2,3,4,7,5,5,5,
0,2,3,4,7,5,5,5,
2,3,4,7,5,5,5,5,
0,1,3,4,7,5,5,5,
1,3,4,7,5,5,5,5,
0,3,4,7,5,5,5,5,
3,4,7,5,5,5,5,5,
0,1,2,4,7,5,5,5,
1,2,4,7,5,5,5,5,
0,2,4,7,5,5,5,5,
2,4,7,5,5,5,5,5,
0,1,4,7,5,5,5,5,
1,4,7,5,5,5,5,5,
0,4,7,5,5,5,5,5,
4,7,5,5,5,5,5,5,
0,1,2,3,7,4,4,4,
1,2,3,7,4,4,4,4,
0,2,3,7,4,4,4,4,
2,3,7,4,4,4,4,4,
0,1,3,7,4,4,4,4,
1,3,7,4,4,4,4,4,
0,3,7,4,4,4,4,4,
3,7,4,4,4,4,4,4,
0,1,2,7,3,3,3,3,
1,2,7,3,3,3,3,3,
0,2,7,3,3,3,3,3,
2,7,3,3,3,3,3,3,
0,1,7,2,2,2,2,2,
1,7,2,2,2,2,2,2,
0,7,1,1,1,1,1,1,
7,1,1,1,1,1,1,1,
0,1,2,3,4,5,6,7,
1,2,3,4,5,6,7,7,
0,2,3,4,5,6,7,7,
2,3,4,5,6,7,7,7,
0,1,3,4,5,6,7,7,
1,3,4,5,6,7,7,7,
0,3,4,5,6,7,7,7,
3,4,5,6,7,7,7,7,
0,1,2,4,5,6,7,7,
1,2,4,5,6,7,7,7,
0,2,4,5,6,7,7,7,
2,4,5,6,7,7,7,7,
0,1,4,5,6,7,7,7,
1,4,5,6,7,7,7,7,
0,4,5,6,7,7,7,7,
4,5,6,7,7,7,7,7,
0,1,2,3,5,6,7,7,
1,2,3,5,6,7,7,7,
0,2,3,5,6,7,7,7,
2,3,5,6,7,7,7,7,
0,1,3,5,6,7,7,7,
1,3,5,6,7,7,7,7,
0,3,5,6,7,7,7,7,
3,5,6,7,7,7,7,7,
0,1,2,5,6,7,7,7,
1,2,5,6,7,7,7,7,
0,2,5,6,7,7,7,7,
2,5,6,7,7,7,7,7,
0,1,5,6,7,7,7,7,
1,5,6,7,7,7,7,7,
0,5,6,7,7,7,7,7,
5,6,7,7,7,7,7,7,
0,1,2,3,4,6,7,7,
1,2,3,4,6,7,7,7,
0,2,3,4,6,7,7,7,
2,3,4,6,7,7,7,7,
0,1,3,4,6,7,7,7,
1,3,4,6,7,7,7,7,
0,3,4,6,7,7,7,7,
3,4,6,7,7,7,7,7,
0,1,2,4,6,7,7,7,
1,2,4,6,7,7,7,7,
0,2,4,6,7,7,7,7,
2,4,6,7,7,7,7,7,
0,1,4,6,7,7,7,7,
1,4,6,7,7,7,7,7,
0,4,6,7,7,7,7,7,
4,6,7,7,7,7,7,7,
0,1,2,3,6,7,7,7,
1,2,3,6,7,7,7,7,
0,2,3,6,7,7,7,7,
2,3,6,7,7,7,7,7,
0,1,3,6,7,7,7,7,
1,3,6,7,7,7,7,7,
0,3,6,7,7,7,7,7,
3,6,7,7,7,7,7,7,
0,1,2,6,7,7,7,7,
1,2,6,7,7,7,7,7,
0,2,6,7,7,7,7,7,
2,6,7,7,7,7,7,7,
0,1,6,7,7,7,7,7,
1,6,7,7,7,7,7,7,
0,6,7,7,7,7,7,7,
6,7,7,7,7,7,7,7,
0,1,2,3,4,5,6,6,
1,2,3,4,5,6,6,6,
0,2,3,4,5,6,6,6,
2,3,4,5,6,6,6,6,
0,1,3,4,5,6,6,6,
1,3,4,5,6,6,6,6,
0,3,4,5,6,6,6,6,
3,4,5,6,6,6,6,6,
0,1,2,4,5,6,6,6,
1,2,4,5,6,6,6,6,
0,2,4,5,6,6,6,6,
2,4,5,6,6,6,6,6,
0,1,4,5,6,6,6,6,
1,4,5,6,6,6,6,6,
0,4,5,6,6,6,6,6,
4,5,6,6,6,6,6,6,
0,1,2,3,5,6,6,6,
1,2,3,5,6,6,6,6,
0,2,3,5,6,6,6,6,
2,3,5,6,6,6,6,6,
0,1,3,5,6,6,6,6,
1,3,5,6,6,6,6,6,
0,3,5,6,6,6,6,6,
3,5,6,6,6,6,6,6,
0,1,2,5,6,6,6,6,
1,2,5,6,6,6,6,6,
0,2,5,6,6,6,6,6,
2,5,6,6,6,6,6,6,
0,1,5,6,6,6,6,6,
1,5,6,6,6,6,6,6,
0,5,6,6,6,6,6,6,
5,6,6,6,6,6,6,6,
0,1,2,3,4,5,5,5,
1,2,3,4,5,5,5,5,
0,2,3,4,5,5,5,5,
2,3,4,5,5,5,5,5,
0,1,3,4,5,5,5,5,
1,3,4,5,5,5,5,5,
0,3,4,5,5,5,5,5,
3,4,5,5,5,5,5,5,
0,1,2,4,5,5,5,5,
1,2,4,5,5,5,5,5,
0,2,4,5,5,5,5,5,
2,4,5,5,5,5,5,5,
0,1,4,5,5,5,5,5,
1,4,5,5,5,5,5,5,
0,4,5,5,5,5,5,5,
4,5,5,5,5,5,5,5,
0,1,2,3,4,4,4,4,
1,2,3,4,4,4,4,4,
0,2,3,4,4,4,4,4,
2,3,4,4,4,4,4,4,
0,1,3,4,4,4,4,4,
1,3,4,4,4,4,4,4,
0,3,4,4,4,4,4,4,
3,4,4,4,4,4,4,4,
0,1,2,3,3,3,3,3,
1,2,3,3,3,3,3,3,
0,2,3,3,3,3,3,3,
2,3,3,3,3,3,3,3,
0,1,2,2,2,2,2,2,
1,7,7,7,7,7,7,7,
0,1,1,1,1,1,1,1,
0,0,0,0,0,0,0,0
};
