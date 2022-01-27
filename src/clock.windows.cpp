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

//|///////////////////// clock_getres ///////////////////////////////////////
extern "C" clock_result clock_getres(uint32_t clockid)
{
  clock_result result = {};

  switch(clockid)
  {
    case clock::id::realtime:
      if (auto err = getres_realtime(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    case clock::id::monotonic:
      if (auto err = getres_monotonic(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    default:
      result.erno = EINVAL;
  }

  return result;
}

//|///////////////////// clock_gettime //////////////////////////////////////
extern "C" clock_result clock_gettime(uint32_t clockid)
{
  clock_result result = {};

  switch(clockid)
  {
    case clock::id::realtime:
      if (auto err = gettime_realtime(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    case clock::id::monotonic:
      if (auto err = gettime_monotonic(&result.timestamp); err < 0)
        result.erno = -err;
      break;

    default:
      result.erno = EINVAL;
  }

  return result;
}
