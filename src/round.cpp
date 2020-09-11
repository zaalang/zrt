//
// round
//

#include <cstddef>
#include <cstdint>

//|///////////////////// roundf /////////////////////////////////////////////
extern "C" float roundf(float x)
{
  constexpr float toint = 1.0 / 1.1920928955078125e-07F;

  float y;
  uint32_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 23 & 0xff);

  if (e >= 0x7f + 23)
    return x;

  if (e < 0x7f - 1)
    return 0.0f * x;

  if ((i >> 31) != 0)
    x = -x;

  y = x + toint - toint - x;

  if (y > 0.5f)
    y = y + x - 1.0f;
  else if (y <= -0.5f)
    y = y + x + 1.0f;
  else
    y = y + x;

  if ((i >> 31) != 0)
    y = -y;

  return y;
}

//|///////////////////// round //////////////////////////////////////////////
extern "C" double round(double x)
{
  constexpr double toint = 1.0 / 2.2204460492503131e-16;

  double y;
  uint64_t i;

  __builtin_memcpy(&i, &x, sizeof(i));

  auto e = (int)(i >> 52 & 0x7ff);

  if (e >= 0x3ff + 52)
    return x;

  if (e < 0x3ff - 1)
    return 0.0f * x;

  if ((i >> 63) != 0)
    x = -x;

  y = x + toint - toint - x;

  if (y > 0.5)
    y = y + x - 1.0;
  else if (y <= -0.5)
    y = y + x + 1.0;
  else
    y = y + x;

  if ((i >> 63) != 0)
    y = -y;

  return y;
}
