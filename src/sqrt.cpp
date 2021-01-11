//
// sqrt
//

#include <cstddef>
#include <cstdint>

#if defined _MSC_VER
extern "C" void* memcpy(void* dst, const void* src, size_t n);
#define __builtin_memcpy memcpy
#pragma intrinsic(memcpy)
#endif

//|///////////////////// sqrtf //////////////////////////////////////////////
extern "C" float sqrtf(float x)
{
  float f;
  uint32_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 23 & 0xff);

  if (x == 0 || (e == 0xff && (x != x || x > 0)))
    return x;

  if (x < 0)
    return __builtin_nanf("");

  if (e == 0)
  {
    while ((i & (1 << 23)) == 0)
    {
      i <<= 1;
      e -= 1;
    }

    e += 1;
  }

  e -= 127;
  i &= 0x807ffffful;
  i |= 0x00800000ul;

  if ((e & 1) == 1)
    i <<= 1;

  e >>= 1;
  i <<= 1;

  auto s = 0ul;
  auto q = 0ul;
  auto r = 0x01000000ul;

  while (r != 0)
  {
    auto t = s + r;
    if (t <= i)
    {
      s = t + r;
      i -= t;
      q += r;
    }

    i <<= 1;
    r >>= 1;
  }

  if (i != 0)
    q += q & 1;

  i = (q >> 1) + (uint32_t(e + 0x7e) << 23);

  __builtin_memcpy(&f, &i, sizeof(f));

  return f;
}

//|///////////////////// sqrt //////////////////////////////////////////////
extern "C" double sqrt(double x)
{
  double d;
  uint64_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 52 & 0x7ff);

  if (x == 0 || (e == 0x7ff && (x != x || x > 0)))
    return x;

  if (x < 0)
    return __builtin_nan("");

  if (e == 0)
  {
    while ((i & (1ull << 52)) == 0)
    {
      i <<= 1;
      e -= 1;
    }

    e += 1;
  }

  e -= 1023;
  i &= 0x800fffffffffffffull;
  i |= 0x0010000000000000ull;

  if ((e & 1) == 1)
    i <<= 1;

  e >>= 1;
  i <<= 1;

  auto s = 0ull;
  auto q = 0ull;
  auto r = 0x0020000000000000ull;

  while (r != 0)
  {
    auto t = s + r;
    if (t <= i)
    {
      s = t + r;
      i -= t;
      q += r;
    }

    i <<= 1;
    r >>= 1;
  }

  if (i != 0)
    q += q & 1;

  i = (q >> 1) + (uint64_t(e + 0x3fe) << 52);

  __builtin_memcpy(&d, &i, sizeof(d));

  return d;
}
