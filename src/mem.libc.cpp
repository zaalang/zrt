//
// mem.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "mem.h"
#include <cstddef>
#include <cstdlib>
#include <errno.h>

#define PAGE_SIZE 4096

//|///////////////////// mem_alloc //////////////////////////////////////////
extern "C" mem_result mem_alloc(uint64_t size)
{
  mem_result result = {};

  size = (size + PAGE_SIZE - 1) & -PAGE_SIZE;

  result.size = size;
  result.addr = malloc(size);

  if (!result.addr)
  {
    result.erno = ENOMEM;
    result.addr = nullptr;
    result.size = 0;
  }

  return result;
}

//|///////////////////// mem_free ///////////////////////////////////////////
extern "C" void mem_free(void const *addr, uint64_t size)
{
  free((void*)addr);
}
