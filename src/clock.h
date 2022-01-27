//
// clock.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#pragma once

#include <cstdint>

namespace clock
{
  enum id
  {
    realtime,
    monotonic,
  };
}

struct clock_result
{
  uint32_t erno;
  uint64_t timestamp;
};

extern "C" clock_result clock_getres(uint32_t clockid);
extern "C" clock_result clock_gettime(uint32_t clockid);
