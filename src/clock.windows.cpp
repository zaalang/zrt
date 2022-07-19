//
// clock.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "clock.h"
#include <windows.h>

#define	EINVAL 22

namespace
{
  int getres_realtime(uint64_t *res)
  {
    *res = 100;

    return 0;
  }

  int getres_monotonic(uint64_t *res)
  {
     LARGE_INTEGER Frequency;

     if (QueryPerformanceFrequency(&Frequency) == 0)
       return -EINVAL;

     *res = (1000000000 + (Frequency.QuadPart >> 1)) / Frequency.QuadPart;

     return 0;
  }

  int gettime_realtime(uint64_t *tp)
  {
    FILETIME filetime;

    GetSystemTimeAsFileTime(&filetime);

    *tp = (((int64_t)filetime.dwHighDateTime << 32) + filetime.dwLowDateTime - 116444736000000000) * 100;

    return 0;
  }

  int gettime_monotonic(uint64_t *tp)
  {
    LARGE_INTEGER frequency;

    if (QueryPerformanceFrequency(&frequency) == 0)
      return -EINVAL;

    LARGE_INTEGER counter;

    if (QueryPerformanceCounter(&counter) == 0)
      return -EINVAL;

    *tp = 1000000000 * (counter.QuadPart / frequency.QuadPart) + ((counter.QuadPart % frequency.QuadPart) * 1000000000 + (frequency.QuadPart >> 1)) / frequency.QuadPart;

    return 0;
  }
}

//|///////////////////// clk_getres /////////////////////////////////////////
extern "C" clk_result clk_getres(uint32_t clockid)
{
  clk_result result = {};

  switch(clockid)
  {
    case clk::id::realtime:
      if (auto err = getres_realtime(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = getres_monotonic(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    default:
      result.erno = EINVAL;
  }

  return result;
}

//|///////////////////// clk_gettime ////////////////////////////////////////
extern "C" clk_result clk_gettime(uint32_t clockid)
{
  clk_result result = {};

  switch(clockid)
  {
    case clk::id::realtime:
      if (auto err = gettime_realtime(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = gettime_monotonic(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    default:
      result.erno = EINVAL;
  }

  return result;
}
