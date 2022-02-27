//
// fmod
//

#include <cstddef>
#include <cstdint>

extern "C" float frexpf(float x, int *exp);
extern "C" double frexp(double x, int *exp);
extern "C" double ldexp(double x, int exp);
extern "C" float ldexpf(float x, int exp);

#if defined _MSC_VER
#define __builtin_isinf(x) ((x) < -1.7976931348623157e+308 || (x) > 1.7976931348623157e+308)
#define __builtin_isnan(x) ((x) != (x))
#endif

//|///////////////////// fmodf //////////////////////////////////////////////
extern "C" float fmodf(float x, float y)
{
  if (y == 0.0f || __builtin_isinf(x) || __builtin_isnan(x) || __builtin_isnan(y))
    return __builtin_nanf("");

  bool sign = false;

  if (x < 0.0f)
  {
    x = -x;
    sign = true;
  }

  if (y < 0.0f)
    y = -y;

  int yexp;
  float yfr = frexpf(y, &yexp);

  while (x >= y)
  {
    int xexp;
    float xfr = frexpf(x, &xexp);

    if (xfr < yfr)
      xexp -= 1;

    x = x - ldexpf(y, xexp - yexp);
  }

  return sign ? -x : x;
}

//|///////////////////// fmod ///////////////////////////////////////////////
extern "C" double fmod(double x, double y)
{
  if (y == 0.0 || __builtin_isinf(x) || __builtin_isnan(x) || __builtin_isnan(y))
    return __builtin_nan("");

  bool sign = false;

  if (x < 0.0)
  {
    x = -x;
    sign = true;
  }

  if (y < 0.0)
    y = -y;

  int yexp;
  double yfr = frexp(y, &yexp);

  while (x >= y)
  {
    int xexp;
    double xfr = frexp(x, &xexp);

    if (xfr < yfr)
      xexp -= 1;

    x = x - ldexp(y, xexp - yexp);
  }

  return sign ? -x : x;
}
