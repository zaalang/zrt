//
// memset
//

#include <cstddef>

//|///////////////////// memset /////////////////////////////////////////////
extern "C" void *memset(void *dst, int ch, size_t n)
{
  auto *d = (unsigned char*)dst;

  for (; n; --n)
    *d++ = ch;

  return dst;
}
