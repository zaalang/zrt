//
// proc.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include <cstdint>
#include <sys/syscall.h>

namespace
{
  long exit(long rval)
  {
    long n = SYS_exit;

    unsigned long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(rval) : "rcx", "r11", "memory");
    return ret;
  }

  long exit_group(long rval)
  {
    long n = SYS_exit_group;

    unsigned long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(rval) : "rcx", "r11", "memory");
    return ret;
  }
}

//|///////////////////// proc_exit //////////////////////////////////////////
extern "C" void proc_exit(uint32_t rval)
{
  exit_group(rval);

  for (;;)
    exit(rval);
}
