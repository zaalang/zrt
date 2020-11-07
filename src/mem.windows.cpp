//
// mem.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "mem.h"
#include <windows.h>

//|///////////////////// mem_alloc //////////////////////////////////////////
extern "C" mem_result mem_alloc(uint64_t size)
{
  mem_result result = {};

#if 0
  SYSTEM_INFO sSysInfo;
  GetSystemInfo(&sSysInfo);
  size = (size + sSysInfo.dwPageSize - 1) & ~(sSysInfo.dwPageSize - 1);
#else
  size = (size + 4096 - 1) & ~(4096 - 1);
#endif

  result.size = size;
  result.addr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

  if (result.addr == NULL)
  {
    result.erno = GetLastError();
    result.size = 0;
  }

  return result;
}

//|///////////////////// mem_free ///////////////////////////////////////////
extern "C" void mem_free(void const *addr)
{
  VirtualFree(const_cast<void*>(addr), 0, MEM_RELEASE);
}
