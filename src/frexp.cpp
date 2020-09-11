//
// frexp
//

#include <cstddef>
#include <cstdint>

//|///////////////////// frexpf /////////////////////////////////////////////
extern "C" float frexpf(float x, int *exp)
{
  float f;
  uint32_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 23 & 0xff);

  if (e == 0)
  {
    if (x != 0.0)
    {
      x = frexpf(x * 0x1p64f, exp);
      *exp -= 64;
    }
    else
    {
      *exp = 0;
    }

    return x;
  }

  if (e == 0xff)
    return x;

  *exp = e - 0x7e;
  i &= 0x807ffffful;
  i |= 0x3f000000ul;

  __builtin_memcpy(&f, &i, sizeof(f));

  return f;
}

//|///////////////////// frexp //////////////////////////////////////////////
extern "C" double frexp(double x, int *exp)
{
  double d;
  uint64_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 52 & 0x7ff);

  if (e == 0)
  {
    if (x != 0.0)
    {
      x = frexp(x * 0x1p64, exp);
      *exp -= 64;
    }
    else
    {
      *exp = 0;
    }

    return x;
  }

  if (e == 0x7ff)
    return x;

  *exp = e - 0x3fe;
  i &= 0x800fffffffffffffull;
  i |= 0x3fe0000000000000ull;

  __builtin_memcpy(&d, &i, sizeof(d));

  return d;
}
