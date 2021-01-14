//
// proc.cpp
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include <cstdint>

#if defined __unix__

#include <sys/syscall.h>

//|///////////////////// exit ///////////////////////////////////////////////
extern "C" void exit(int rval)
{
  {
    long n = SYS_exit_group;

    unsigned long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(rval) : "rcx", "r11", "memory");
  }

  for (;;)
  {
    long n = SYS_exit;

    unsigned long ret;
    asm volatile ("syscall" : "=a"(ret) : "0"(n), "D"(rval) : "rcx", "r11", "memory");
  }
}

#endif

#if defined _WIN32

#include <windows.h>

//|///////////////////// exit ///////////////////////////////////////////////
extern "C" void exit(int rval)
{
  ExitProcess(rval);
}

#endif
