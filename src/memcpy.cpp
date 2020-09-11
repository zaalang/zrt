//
// memcpy
//

#include <cstddef>

//|///////////////////// memcpy /////////////////////////////////////////////
extern "C" void *memcpy(void * __restrict__ dst, const void * __restrict__ src, size_t n)
{
  auto *d = (unsigned char*)dst;
  auto *s = (const unsigned char*)src;

  for (; n; --n)
    *d++ = *s++;

  return dst;
}
