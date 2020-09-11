//
// memmove
//

#include <cstddef>

//|///////////////////// memmove ////////////////////////////////////////////
extern "C" void *memmove(void *dst, const void *src, size_t n)
{
  auto *d = (unsigned char*)dst;
  auto *s = (const unsigned char*)src;

  if (d == s)
    return d;

  if (d < s)
  {
    for (; n; --n)
      *d++ = *s++;
  }
  else
  {
    while (n)
    {
      --n;
      d[n] = s[n];
    }
  }

  return dst;
}
