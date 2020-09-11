//
// trunc
//

#include <cstddef>
#include <cstdint>

//|///////////////////// truncf /////////////////////////////////////////////
extern "C" float truncf(float x)
{
  float f;
  uint32_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 23 & 0xff) - 0x7f + 9;

  if (e < 9)
    e = 1;

  if (e >= 23 + 9)
    return x;

  auto m = uint32_t(-1) >> e;

  if ((i & m) == 0)
    return x;

  i &= ~m;

  __builtin_memcpy(&f, &i, sizeof(f));

  return f;
}

//|///////////////////// trunc //////////////////////////////////////////////
extern "C" double trunc(double x)
{
  double d;
  uint64_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 52 & 0x7ff) - 0x3ff + 12;

  if (e < 12)
    e = 1;

  if (e >= 52 + 12)
    return x;

  auto m = uint64_t(-1) >> e;

  if ((i & m) == 0)
    return x;

  i &= ~m;

  __builtin_memcpy(&d, &i, sizeof(d));

  return d;
}
