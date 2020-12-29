//
// floor
//

#include <cstddef>
#include <cstdint>

#ifdef _MSC_VER
extern "C" void* memcpy(void* dst, const void* src, size_t n);
#define __builtin_memcpy memcpy
#pragma intrinsic(memcpy)
#endif

//|///////////////////// floorf /////////////////////////////////////////////
extern "C" float floorf(float x)
{
  float f;
  uint32_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 23 & 0xff) - 0x7f;

  if (e < 0)
  {
    if ((i >> 31) == 0)
      return 0.0f;

    if ((i << 1) != 0)
      return -1.0f;

    return -0.0f;
  }

  if (e >= 23)
    return x;

  auto m = 0x007fffff >> e;

  if ((i & m) == 0)
    return x;

  if ((i >> 31) != 0)
    i += m;

  i &= ~m;

  __builtin_memcpy(&f, &i, sizeof(f));

  return f;
}

//|///////////////////// floor //////////////////////////////////////////////
extern "C" double floor(double x)
{
  constexpr double toint = 1.0 / 2.2204460492503131e-16;

  double y;
  uint64_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 52 & 0x7ff);

  if (e >= 0x3ff + 52 || x == 0.0)
    return x;

  if ((i >> 63) != 0)
    y = x - toint + toint - x;
  else
    y = x + toint - toint - x;

  if (e <= 0x3ff - 1)
    return (i >> 63) ? -1.0 : 0.0;

  if (y > 0)
    return x + y - 1.0;

  return x + y;
}
