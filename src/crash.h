//
// crash.h
//
// Copyright (C) Peter Niekamp
//
// This code remains the property of the copyright holder.
// The code contained herein is licensed for use without limitation
//

#include "fd.h"
#include "proc.h"

static inline void crash(const char *msg, int len)
{
  ciovec io;
  io.data = (uint8_t const *)msg;
  io.len = len;

  fd_writev(STDERR, &io, 1);

#ifndef _MSC_VER
  asm volatile ("int $0x03");
#endif

  exit(1);
}
