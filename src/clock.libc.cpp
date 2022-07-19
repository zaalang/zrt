//
// clock.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "clock.h"
#include <time.h>
#include <errno.h>

//|///////////////////// clk_getres /////////////////////////////////////////
extern "C" clk_result clk_getres(uint32_t clockid)
{
  clk_result result = {};

  timespec res;

  switch(clockid)
  {
    case clk::id::realtime:
      if (auto err = clock_getres(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = clock_getres(CLOCK_MONOTONIC, &res); err < 0)
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
      if (auto err = clock_gettime(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clk::id::monotonic:
      if (auto err = clock_gettime(CLOCK_MONOTONIC, &res); err < 0)
        result.erno = -err;
      break;

    default:
      res = {};
      result.erno = EINVAL;
  }

  result.timestamp = static_cast<uint64_t>(res.tv_sec) * 1000000000 + res.tv_nsec;

  return result;
}
