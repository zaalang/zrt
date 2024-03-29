//
// clock.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "clock.h"
#include <sys/types.h>
#include <sys/syscall.h>

#define	EFAULT 14
#define	EINVAL 22

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

int (*vdso_clock_getres)(uint32_t, timespec*);
int (*vdso_clock_gettime)(uint32_t, timespec*);

namespace
{
  int getres(int clockid, timespec *res)
  {
    if (!vdso_clock_getres)
      return -EFAULT;

    return vdso_clock_getres(clockid, res);
  }

  int gettime(int clockid, timespec *tp)
  {
    if (!vdso_clock_gettime)
      return -EFAULT;

    return vdso_clock_gettime(clockid, tp);
  }
}

//|///////////////////// clk_getres /////////////////////////////////////////
extern "C" clk_result clk_getres(uint32_t clockid)
{
  clk_result result = {};

  timespec res;

  switch(clockid)
  {
    case clk::id::realtime:
      if (auto err = getres(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = getres(CLOCK_MONOTONIC, &res); err < 0)
        result.erno = -err;
      break;

    default:
      res = {};
      result.erno = EINVAL;
  }

  result.timestamp = static_cast<uint64_t>(res.tv_sec) * 1000000000 + res.tv_nsec;

  return result;
}

//|///////////////////// clk_gettime ////////////////////////////////////////
extern "C" clk_result clk_gettime(uint32_t clockid)
{
  clk_result result = {};

  timespec res;

  switch(clockid)
  {
    case clk::id::realtime:
      if (auto err = gettime(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = gettime(CLOCK_MONOTONIC, &res); err < 0)
        result.erno = -err;
      break;

    default:
      res = {};
      result.erno = EINVAL;
  }

  result.timestamp = static_cast<uint64_t>(res.tv_sec) * 1000000000 + res.tv_nsec;

  return result;
}
