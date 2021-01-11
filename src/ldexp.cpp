//
// ldexp
//

#include <cstddef>
#include <cstdint>

#if defined _MSC_VER
extern "C" void* memcpy(void* dst, const void* src, size_t n);
#define __builtin_memcpy memcpy
#pragma intrinsic(memcpy)
#endif

//|///////////////////// ldexpf /////////////////////////////////////////////
extern "C" float ldexpf(float x, int exp)
{
  float y = x;

  if (exp > 127)
  {
    y *= 0x1p127f;
    exp -= 127;

    if (exp > 127)
    {
      y *= 0x1p127f;
      exp -= 127;

      if (exp > 127)
        exp = 127;
    }
  }
  else if (exp < -126)
  {
    y *= 0x1p-126f * 0x1p24f;
    exp += 126 - 24;

    if (exp < -126)
    {
      y *= 0x1p-126f * 0x1p24f;
      exp += 126 - 24;

      if (exp < -126)
        exp = -126;
    }
  }

  float f;
  uint32_t i;

  i = (uint32_t)(0x7f + exp) << 23;

  __builtin_memcpy(&f, &i, sizeof(f));

  return y * f;
}

//|///////////////////// ldexp //////////////////////////////////////////////
extern "C" double ldexp(double x, int exp)
{
  double y = x;

  if (exp > 1023)
  {
    y *= 0x1p1023;
    exp -= 1023;

    if (exp > 1023)
    {
      y *= 0x1p1023;
      exp -= 1023;

      if (exp > 1023)
        exp = 1023;
    }
  }
  else if (exp < -1022)
  {
    y *= 0x1p-1022 * 0x1p53;
    exp += 1022 - 53;

    if (exp < -1022)
    {
      y *= 0x1p-1022 * 0x1p53;
      exp += 1022 - 53;

      if (exp < -1022)
        exp = -1022;
    }
  }

  double d;
  uint64_t i;

  i = (uint64_t)(0x3ff + exp) << 52;

  __builtin_memcpy(&d, &i, sizeof(d));

  return y * d;
}
