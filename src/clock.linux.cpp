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

#define	EINVAL 22

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

namespace
{
  int getres(int clockid, timespec *res)
  {
    long n = SYS_clock_getres;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(clockid), "S"(res) : "rcx", "r11", "memory");
    return ret;
  }

  int gettime(int clockid, timespec *tp)
  {
    long n = SYS_clock_gettime;

    int ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(clockid), "S"(tp) : "rcx", "r11", "memory");
    return ret;
  }
}

//|///////////////////// clock_getres ///////////////////////////////////////
extern "C" clock_result clock_getres(uint32_t clockid)
{
  clock_result result = {};

  timespec res;

  switch(clockid)
  {
    case clock::id::realtime:
      if (auto err = getres(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clock::id::monotonic:
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

//|///////////////////// clock_gettime //////////////////////////////////////
extern "C" clock_result clock_gettime(uint32_t clockid)
{
  clock_result result = {};

  timespec res;

  switch(clockid)
  {
    case clock::id::realtime:
      if (auto err = gettime(CLOCK_REALTIME, &res); err < 0)
        result.erno = -err;
      break;

    case clock::id::monotonic:
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
