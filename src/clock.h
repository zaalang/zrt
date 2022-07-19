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

namespace clk
{
  enum id
  {
    realtime,
    monotonic,
  };
}

struct clk_result
{
  uint32_t erno;
  uint64_t timestamp;
};

extern "C" clk_result clk_getres(uint32_t clockid);
extern "C" clk_result clk_gettime(uint32_t clockid);
