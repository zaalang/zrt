//
// mem.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#pragma once

#include <cstdint>

struct mem_result
{
  uint32_t erno;
  uint64_t size;
  void *addr;
};

extern "C" mem_result mem_alloc(uint64_t size);
extern "C" void mem_free(void const *addr);
